## `NDRtoSVS`: Tools to run a PTN as C program using System V Sematphores (ONLY!)

# Content: 

`NDRtoSVS` -- Tina plugin to export an PTN from the graphic editor `nd` as System V Semaphores in a C language header (.h)

`ptn_svs` and `ptn_svs_fork` -- two variants of SVS machine - one using threads, the other using processess, correspondingly; data of a chosen PTN is inserted as .h file

`add.ndr` -- addition inhibitor PTN in graphical format of Tina

`add.h` -- addition inhibitor PTN as SVS data in .h file

`mul.ndr` -- multiplication inhibitor PTN in graphical format of Tina

`mul.h` -- multiplication inhibitor PTN as SVS data in .h file

`NDRtoSVS.tcl`-- format of pop-up window for Tina's `nd` plugins

`ndmount+.txt` -- lines to add to `ndmount` file of Tina's `n` to attach NDRtoSVS pluging


# Attach NDRtoSVS to Tina `nd`

Compile `NDRtoSVS` converter: gcc -o NDRtoSVS NDRtoSVS.c

Copy binary file `NDRtoSVS` to `tina/bin` directory

Copy `NDRtoSVS.tcl` file to `tina/bin/plugins` directory

Copy lines from `ndmount+.txt` file and append them to `tina/bin/plugins/ndmount`

Run `nd`


# Work with PTN

Draw a net in Tina `nd` and save it as an .ndr file, for example add.ndr

In `nd` choose from menue: Tools - convert PTN specification - `.h-SVS`

Save the resulting file as .h, for example `add.h`

Correct the name of included data file within `ptn_svs.c`, for example to add.h (#inlude "add.h")

Compile SVS machine: gcc -o ptn_svs ptn_svs.c

Run SVS machine: ./ptn_svs

Options of `ptn_svs` -- first parameter -- level of output detail: 0, 1, 2

In a similar way ptn_svs_fork.c is compiled and run. 


# Data formats compatibility:

`Tina`, `nd`, and `NDR` file format according to https://projects.laas.fr/tina/index.php

`SVS` Unix/Linux application usung System V semaphores (ONLY!) according to [2]

   
References: 
----------- 
1. Zaitsev, D. A. (2026). Computing on semaphores of Dijkstra with place-transition nets. International Journal of Parallel, Emergent and Distributed Systems, 1–30. https://doi.org/10.1080/17445760.2026.2615010
2. AT&T. System V interface definition (Vols. I & II). Indianapolis: AT&T; 1986.
3. libc
