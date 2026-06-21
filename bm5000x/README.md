## Peak-load benchmarks for operating systems: Linux runs over 5,000× slower than our virtual machines ##

We applied place-transition nets (PTNs) defined by System V semaphores (https://doi.org/10.1080/17445760.2026.2615010) to benchmark Linux (Ubuntu 24.04.4 LTS, kernel 6.17.0-35).

Using PTNs for matrix multiplication and arrays of concurrent multiplications, we compared Linux kernel performance with that of our virtual machines (https://doi.org/10.1080/17445760.2025.2490148).

A PTN executing 1,024 parallel multiplications of 6-bit data completed in **0.912 seconds** on our VM, compared with **5,673.597 seconds** on Linux running on the same hardware (AMD Ryzen 7 6800H @ 4.8 GHz, 32 GB RAM). The application contains **9,216 semaphores (places)** and **8,192 processes (transitions)**.

The Linux execution time is more than **5,000 times slower** than that of our VM. We believe this gap cannot be explained solely by system-call and context-switching overhead. Instead, it points to the efficiency of the System V semaphore implementation in the Linux `sem.c` kernel module.

We are interested in collaborating on projects aimed at implementing semaphores with **wait-for-all semantics** in Linux, both at the kernel level for processes and as a runtime mechanism for fast, futex-like thread synchronization. While `futex_waitv` provides wait-for-any semantics, wait-for-all semantics could help eliminate many deadlocks caused by sequential resource acquisition.

For modeling, we use Tina (https://projects.laas.fr/tina/index.php) as an IDE and generate large PTN models with our own toolchains. Models are exported through our NDRtoALL plugin as `.h` files for the PVZ machine, then recompiled and executed as Linux applications.

Our basic tools are available on GitHub: https://github.com/dimazaitsev/SNCtools

# Mind early alpha versions

# watch benchmark net in Tina nd - open .ndr file (too big to handle)
Have a look at `gm6x6-a4.pdf` to understand the benchmark

# Benchmark Linux kernel
    gcc -O3 -o pvzm pvzm.c
    ./pvzm

# Benchmark WN (SalWicki Net) VM 
    gcc -O3 -o wn-bm wn-bm.c
    ./wn-bm 1 < gm16x16-b6.mcc

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* 16x16=256 concurrent multiplications
gm16x16-b6 (118x118=13924)
289.010659 s vs 0.368973 s
* 32x32=1024 concurrent multiplication
gm32x32-b6 ((118x118=13924))
5673.596540 s vs 0.912282
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~




