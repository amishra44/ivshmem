# ivshmem Host Server
## Start ivshmem server

`ivshmem-server -F -v`

If server doesn't start, delete the shared memory file and retry

`rm /tmp/ivshmem_socket`

# ivshmem Guest (VM) Driver and Example
## Build Source Code
`make`

## Load Driver
`sudo insmod ne_ivshmem_ldd_basic.ko`

## Run Example
Write to shared memory (first VM)

`./ne_ivshmem_shm_guest_usr -w hello`

Read from shared memory (second VM)

`./ne_ivshmem_shm_guest_usr -r`

`hello`
