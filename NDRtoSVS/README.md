## `NDRtoSVS`: Tools to run a PTN as C program using System V Sematphores (ONLY!)

# Content: 

`NDRtoSVS` -- Tina plugin to export an PTN from the graphic editor `nd` as System V Semaphores in a C language header (.h)

`ptn_svs` -- Tina plugin to export an SN from the graphic editor `nd` in LSN/HSN format or as C language header (.h)

`add.ndr` -- SN file convertor: LSN to MCC

`add.h` -- SN virtual machine for multicore CPU with input in MCC format

`mul.ndr` -- SN virtual machine for GPU with input in MCC format

`mul.h` -- simulation and prototyping within FreeRTOS, automated manufacture shop case study


# Data formats compatibility:

`Tina`, `nd`, and `NDR` file format according to https://projects.laas.fr/tina/index.php

`SVS` Unix/Linux application usung System V semaphores (ONLY!) according to [2]



   
References: 
----------- 
1. Zaitsev, D. A. (2026). Computing on semaphores of Dijkstra with place-transition nets. International Journal of Parallel, Emergent and Distributed Systems, 1–30. https://doi.org/10.1080/17445760.2026.2615010
2. AT&T. System V interface definition (Vols. I & II). Indianapolis: AT&T; 1986.
3. libc
