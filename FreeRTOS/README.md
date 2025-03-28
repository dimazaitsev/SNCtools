# `SNCtools`: Tools for Sleptsov Net Computing (see refs and https://dimazaitsev.github.io/snc.html)
# RACS: Robotic Arm Control Simulator, FreeRTOS, Linux


## Content 

`es-ra-freertos.c` Robotic Arm Control Simulator (RACS), FreeRTOS, Linux

For further implementation on microcontrollers, say on Raspberry Pi Pico, using Pico SDK, etc., when perlacing the plant model by interaction with an actual plant. 


## HowTo

The automated manufacture shop composition of a robotic arm, storage (palette), and rotary table is specified in:

Zaitsev D.A. Petri Nets and Modelling of Systems: textbook for laboratory training. Odessa: ONAT, 2007, 42 p. In Ukr. / Eng. -- https://dimazaitsev.github.io/pdf/pnms-en.pdf

Laboratory lesson 6. Construction and investigation of models for automatic control 

The control algorithm is given by a Sleptsov Net (https://dimazaitsev.github.io/snc.html)

Run using FreeRTOS Linux port: 

1) Install FreeRTOS Linux port https://bitbucket.org/fjrg76/freertosv202107.00_linux_port_only/src/master/README.tx
2) replace code inside main_blinky.c by this code
4) make
5) ./build/posix_demo

Sleptsov steers !


# Learn when coding

1) Compose autonomous models for other parts of the automated manufacture shop: storage (palette) and rotary table
2) Compose an integrated model of the entire shop
3) Bring in hardware -- replace by hardware models of plant


# Alternative implementation - using Sleptsov Net Computing

1) Draw ES in Tina, nd graphical editor
2) Verify ES in Tina
3) Export ES as .h file for Arduino SN VM
4) Compile SN VM with obtained .h file
5) Upload code to a microcontroller

   
## References
 
1. Ruiyao Xu, Si Zhang, Ding Liu, and Dmitry A. Zaitsev, Sleptsov net based reliable embedded system design on microcontrollers and FPGAs, Proc. of 2024 IEEE International Conference on Embedded Software and Systems (ICESS), Wuhan, China, Dec. 13-15, 2024. https://doi.org/10.1109/ICESS64277.2024.00011
2. Dmitry A. Zaitsev, Tatiana R. Shmeleva, Qing Zhang, and Hongfei Zhao, Virtual Machine and Integrated Developer Environment for Sleptsov Net Computing Parallel Processing Letters, Vol. 33, No. 03, 2350006 (2023). https://doi.org/10.1142/S0129626423500068
3. Zaitsev D.A., Jürjens J. Programming in the Sleptsov net language for systems control, Advances in Mechanical Engineering, 2016, Vol. 8(4), 1-11. https://doi.org/10.1177%2F1687814016640159
4. Zaitsev D.A. Sleptsov Nets Run Fast, IEEE Transactions on Systems, Man, and Cybernetics: Systems, 2016, Vol. 46, No. 5, 682 - 693. http://dx.doi.org/10.1109/TSMC.2015.2444414
5. Zaitsev D.A. Petri Nets and Modelling of Systems: textbook for laboratory training. Odessa: ONAT, 2007, 42 p. In Ukr. / Eng. https://dimazaitsev.github.io/pdf/pnms-en.pdf

----------------------------------------------------------------------- 
@ 2025 Dmitry Zaitsev: daze@acm.org 
