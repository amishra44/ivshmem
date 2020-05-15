# ivshmem Driver and Example
## Building Source Code
`make`

## Loading Driver
`sudo insmod ne_ivshmem_ldd_basic.ko`

## Running Example
Write to shared memory (first VM)

`./ne_ivshmem_shm_guest_usr -w hello`

Read from shared memory (second VM)

`./ne_ivshmem_shm_guest_usr -r`

`hello`
