## `NDRtoSVS`: Tools to run a PTN as C program using System V Sematphores (ONLY!)

# Content: 

`NDRtoSVS` -- Tina plugin to export an PTN from the graphic editor `nd` as System V Semaphores in a C language header (.h)

`ptn_svs` -- SVS machine, data inserted as .h file

`add.ndr` -- addition inhibitor PTN in graphical format of Tina

`add.h` -- addition inhibitor PTN as SVS data in .h file

`mul.ndr` -- multiplication inhibitor PTN in graphical format of Tina

`mul.h` -- multiplication inhibitor PTN as SVS data in .h file


# Data formats compatibility:

`Tina`, `nd`, and `NDR` file format according to https://projects.laas.fr/tina/index.php

`SVS` Unix/Linux application usung System V semaphores (ONLY!) according to [2]



   
References: 
----------- 
1. Zaitsev, D. A. (2026). Computing on semaphores of Dijkstra with place-transition nets. International Journal of Parallel, Emergent and Distributed Systems, 1–30. https://doi.org/10.1080/17445760.2026.2615010
2. AT&T. System V interface definition (Vols. I & II). Indianapolis: AT&T; 1986.
3. libc
