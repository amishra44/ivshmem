/*
 * file : ne_ivshmem_shm_guest_usr.c
 * desc : a demo program that updates/reads the ivshmem BAR2 MMIO region
 *
 * Siro Mugabi, Copyright (c) nairobi-embedded.org, GPLv2
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define prfmt(fmt) "%s:%d:: " fmt, __func__, __LINE__
#define prinfo(fmt, ...) printf(prfmt(fmt), ##__VA_ARGS__)
#define prerr(fmt, ...) fprintf(stderr, prfmt(fmt), ##__VA_ARGS__)
#ifdef DEBUG
#define prdbg_p(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define prdbg_p(fmt, ...) do{}while(0)
#endif

struct ivshmem_data {
	const char *filename;
	ssize_t filesize;
	enum {
		NE_IVSHMEM_READ,
		NE_IVSHMEM_WRITE,
	} ivshmem_op;
};

static const char *prog;

static void usage(void)
{
	fprintf(stderr, "\n"
		"Usage: %s [OPTS] [STRING]" "\n" "\n"
		"OPTS:\n"
		"-f,--filename    IVSHMEM special device filename, e.g. \"/dev/ivshmem0\"\n"
		"-s,--filesize    IVSHMEM MMIO region size, e.g. 0x100000\n"
		"-w,--write       Specifies a write operation\n"
		"-r,--read        Specifies a read operation\n" "\n", prog);
	fprintf(stderr,
		"STRING:          A text string; REQUIRED for write operations\n"
		"\n");
	exit(EXIT_FAILURE);
}

static void bad_option(const char *option)
{
	fprintf(stderr, "\n\tBad option \"%s\"\n", option);
	usage();
}

int main(int argc, char **argv)
{
	struct option long_option[] = {
		{"help", 0, NULL, 'h'},
		{"write", 0, NULL, 'w'},
		{"read", 0, NULL, 'r'},
		{"filename", 1, NULL, 'f'},
		{"filesize", 1, NULL, 's'},
		{NULL, 0, NULL, 0},
	};
	int fd;
	void *map = NULL;
	const char *usrstrng = NULL, *filename = NULL;
	ssize_t filesize = 0;
	struct ivshmem_data ivd;
	char *eptr;

	if (!(prog = strdup(argv[0]))) {
		prerr("strdup(3)\n");
		exit(EXIT_FAILURE);
	}

	ivd.filename = "/dev/ivshmem0";	/* default '/dev' node */
	ivd.filesize = 0x100000;	/* default mmio region size */
	ivd.ivshmem_op = NE_IVSHMEM_READ;	/* default op */

	while (1) {
		int c;
		if ((c =
		     getopt_long(argc, argv, "hwrf:s:", long_option, NULL)) < 0)
			break;
		switch (c) {

		case 'w':
			ivd.ivshmem_op = NE_IVSHMEM_WRITE;
			break;

		case 'r':
			ivd.ivshmem_op = NE_IVSHMEM_READ;
			break;

		case 'f':
			if (!(ivd.filename = strdup(optarg))) {
				prerr("strdup(3)\n");
				exit(EXIT_FAILURE);
			}
			break;

		case 's':
			ivd.filesize = strtoul(optarg, &eptr, 0);
			if (*eptr != '\0')
				bad_option("filesize");
			break;

		case 'h':
		default:
			usage();
			break;
		}
	}
	filename = ivd.filename;
	filesize = ivd.filesize;

	if (ivd.ivshmem_op == NE_IVSHMEM_WRITE) {
		if (optind >= argc) {
			fprintf(stderr, "\n"
				"\t!! Please specify string to write into mmio region !!\n");
			usage();
		}
		usrstrng = argv[optind];
	}

	prdbg_p
	    ("\nYou entered: filename = \"%s\", filesize = %d, operation = %d",
	     filename, (int)filesize, ivd.ivshmem_op);
	if (ivd.ivshmem_op == NE_IVSHMEM_WRITE)
		prdbg_p(", output_string = \"%s\"\n\n", usrstrng);
	else
		prdbg_p("\n\n");

	if ((fd = open(filename, O_RDWR)) < 0) {
		prerr("%s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	if ((map =
	     mmap(0, filesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
		  0)) == (caddr_t) - 1) {
		fprintf(stderr, "%s\n", strerror(errno));
		close(fd);
		exit(EXIT_FAILURE);
	}

	switch (ivd.ivshmem_op) {
	case NE_IVSHMEM_READ:
		if (filesize)
			prinfo("read \"%s\"\n", (char *)map);
		break;

	case NE_IVSHMEM_WRITE:
		prinfo("writing \"%s\"\n", usrstrng);
		strcpy((char *)map, usrstrng);
		break;

	default:
		prinfo("no read/write operations performed\n");
	}

	if ((munmap(map, filesize)) < 0)
		prerr("WARNING: Failed to munmap \"%s\"\n", filename);

	close(fd);

	return 0;
}
