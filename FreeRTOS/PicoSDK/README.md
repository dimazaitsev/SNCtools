# RACPico: Robotic Arm Control prototype: FreeRTOS, RP Pico SDK, LED, Keypad

## Preliminaries

The automated manufacture shop composition of a robotic arm, storage (palette), and rotary table is specified in:

Zaitsev D.A. Petri Nets and Modelling of Systems: textbook for laboratory training. Odessa: ONAT, 2007, 42 p. In Ukr. / Eng. https://dimazaitsev.github.io/pdf/pnms-en.pdf

The control algorithm is given by a Sleptsov Net (https://dimazaitsev.github.io/snc.html)

We use LCD to indicate actuators and sensors and keypad to input sensor. 

Connection: Keypad GPIO 13, 12, 11, 10, 9, 8, 7, 6; LCD GPIO 4 sda, 5 scl and 3.3v, gnd (see photo)

## Video

https://youtu.be/7HPnhaNGyqo

## Install & run

Install and run software in FreeRTOS RP Pico port:
https://github.com/aws-iot-builder-tools/freertos-pi-pico

1) save this file as main.c to app
2) add the line to CMakeLists.txt: target_link_libraries(app pico_stdlib hardware_i2c)
3) cd build; cmake ..; make
4) upload app.uf2 to pico
5) Input sensors corresponding to actuators completed by #
6) Check the model trace appended at the bottom

To jump to the spot: (mind your name for "pisdk":)
cd /home/sysprog/pisdk/freertos-pi-pico/app/build

## Excuses
Please excuse for a clumsy insertion of code (to have a single .c) for:
Keypad from https://github.com/hhoswaldo/pico-keypad4x4
LCD from https://github.com/raspberrypi/pico-examples/tree/master/i2c/lcd_1602_i2c

## Sleptsov steers !

References for Sleptsov Net Computing (SNC) to read, watch, run, cite, and join:
https://dimazaitsev.github.io/snc.html
