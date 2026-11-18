## Peak-load benchmarks for operating systems: Linux runs over 5,000× slower than our virtual machines ##

We applied place-transition nets (PTNs) defined by System V semaphores (https://doi.org/10.1080/17445760.2026.2615010) to benchmark Linux (Ubuntu 24.04.4 LTS, kernel 6.17.0-35).

Using PTNs for matrix multiplication and arrays of concurrent multiplications, we compared Linux kernel performance with that of our virtual machines (https://doi.org/10.1080/17445760.2025.2490148).

A PTN executing 1,024 parallel multiplications of 6-bit data completed in **0.912 seconds** on our VM, compared with **5,673.597 seconds** on Linux running on the same hardware (AMD Ryzen 7 6800H @ 4.8 GHz, 32 GB RAM). The application contains **9,216 semaphores (places)** and **8,192 processes (transitions)**.

The Linux execution time is more than **5,000 times slower** than that of our VM. We believe this gap cannot be explained solely by system-call and context-switching overhead. Instead, it points to the efficiency of the System V semaphore implementation in the Linux `sem.c` kernel module.

We are interested in collaborating on projects aimed at implementing semaphores with **wait-for-all semantics** in Linux, both at the kernel level for processes and as a runtime mechanism for fast, futex-like thread synchronization. While `futex_waitv` provides wait-for-any semantics, wait-for-all semantics could help eliminate many deadlocks caused by sequential resource acquisition.

For modeling, we use Tina (https://projects.laas.fr/tina/index.php) as an IDE and generate large PTN models with our own toolchains. Models are exported through our NDRtoALL plugin as `.h` files for the PVZ machine, then recompiled and executed as Linux applications.

Our basic tools are available on GitHub: https://github.com/dimazaitsev/SNCtools

### Mind early alpha v3 versions with the following changes

- "PTN SVS machine" replaced by "PVZ machine"  
- Sparse matrix data format for sops of all semop()  
- Default scarse output to use in scripts  

### Watch benchmark net in Tina nd - open .ndr file 

- Too big to handle.  
- Have a look at `gm6x6-a4.pdf` to understand the benchmark.  

### Benchmark Linux kernel

Edit PVZ data file name in `#include` within `pvzm.c`

    >gcc -O3 -o pvzm pvzm.c
    >./pvzm

Alternative implementation with threads

    >gcc -O3 -o pvzm_thread pvzm_thread.c -pthread
    >./pvzm_thread 1

A parameter specifies the output detail level

### Benchmark WN (SalWicki Net) VM 

    >gcc -O3 -o wn-bm wn-bm.c
    >./wn-bm 1 < gm16x16-b6.mcc

### Plot of the benchmark

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* 16x16=256 concurrent multiplications, 6-bits numbers
gm16x16-b6 (118x118=13924)
289.010659 s vs 0.368973 s
* 32x32=1024 concurrent multiplication, 6-bits numbers
gm32x32-b6 ((118x118=13924))
5673.596540 s vs 0.912282
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Slow incremental multiplication. Dependence on the number of bits influences more than the matrix size.


### Inspired benchmarks

We are glad that new benchmarks started to appear, which **emulate PVZ machine and run on it our benchmarks** to scrutinize other subsystems of Linux kernel, in particular Inter-Process (Inter-Thread) Communication facilities: 

* Tatiana Shmeleva, Peak-load benchmarks for POSIX semaphores in Linux kernel, https://github.com/tishtri/ConcCom/tree/main/bm_posem


### References

1) Zaitsev, D. A. (2026). Computing on semaphores of Dijkstra with place-transition nets. International Journal of Parallel, Emergent and Distributed Systems, 41(3), 291–320. https://doi.org/10.1080/17445760.2026.2615010
3) Zaitsev, Dmitry and Naluba, Richard, PVZ Machine as a Formal System Based on Unix-Linux fork() and semop() System Calls (July 08, 2026). Preprint available at SSRN: http://dx.doi.org/10.2139/ssrn.7606379
5) Zaitsev, D. A., Ajima, Y., Bartlett, J. F. C., & Kumar, A. (2025). 3D multicore CPU vs GPU on sparse patterns of Sleptsov net virtual machine. International Journal of Parallel, Emergent and Distributed Systems, 40(4), 352–372. https://doi.org/10.1080/17445760.2025.2490148

[1] is obsoleted by [2] for PVZ machine specification; 
[3] describes independent virtual machins to run PTNs directly

