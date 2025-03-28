// RACS: Robotic Arm Control Simulator, FreeRTOS, Linux
//
// The automated manufacture shop composition of a robotic arm, storage (palette), and rotary table is specified in:
// Zaitsev D.A. Petri Nets and Modelling of Systems: textbook for laboratory training. Odessa: ONAT, 2007, 42 p. In Ukr. / Eng.
// https://dimazaitsev.github.io/pdf/pnms-en.pdf
// The control algorithm is given by a Sleptsov Net (https://dimazaitsev.github.io/snc.html)
// 
// Run using FreeRTOS Linux port: 
// https://bitbucket.org/fjrg76/freertosv202107.00_linux_port_only/src/master/README.txt
// 1) replace code inside main_blinky.c by this code
// 2) make
// 3) ./build/posix_demo
//
// Sleptsov steers !

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "console.h"

QueueHandle_t S, A; // modeling Sensors and Actuators

// encoding of sensors

#define StorageIsReady			0x0001
#define DetailIsReady			0x0002
#define ThereIsLeftTurnOfLever		0x0004
#define ThereIsRightTurnOfLever		0x0008
#define ThereIsLeftTurnOfHead		0x0010
#define ThereIsRightTurnOfHead		0x0020
#define ThereIsLowerPositionOfPole	0x0040
#define ThereIsUpperPositionOfPole	0x0080
#define Claw1Grasped			0x0100
#define Claw2Grasped			0x0200
#define Claw1IsOpen			0x0400
#define Claw2IsOpen			0x0800

char * SensorName[] = {
  "StorageIsReady",
  "DetailIsReady",
  "ThereIsLeftTurnOfLever",
  "ThereIsRightTurnOfLever",
  "ThereIsLeftTurnOfHead",
  "ThereIsRightTurnOfHead",
  "ThereIsLowerPositionOfPole",
  "ThereIsUpperPositionOfPole",
  "Claw1Grasped",
  "Claw2Grasped",
  "Claw1IsOpen",
  "Claw2IsOpen" 
};

// encoding of actuators

#define NextToStorage			0x0001
#define BilletIsDelivered		0x0002
#define RotateLeverToTheLeft		0x0004
#define RotateLeverToTheRight		0x0008
#define RotateHeadToTheLeft		0x0010
#define RotateHeadToTheRight		0x0020
#define LowerPoleDown			0x0040
#define RaisePoleUp			0x0080
#define GraspByClaw1			0x0100
#define GraspByClaw2			0x0200
#define OpenClaw1			0x0400
#define OpenClaw2			0x0800

char * ActuatorName[] = {
  "NextToStorage",
  "BilletIsDelivered",
  "RotateLeverToTheLeft",
  "RotateLeverToTheRight",
  "RotateHeadToTheLeft",
  "RotateHeadToTheRight",
  "LowerPoleDown",
  "RaisePoleUp",
  "GraspByClaw1",
  "GraspByClaw2",
  "OpenClaw1",
  "OpenClaw2" 
};

char * MaskToNames(char * prefix, int16_t mask, char * names[], char *buf ) {
  int i=0;
  char *s=buf; s[0]='\0';
  strcat(s,prefix);
  while( mask!=0 ) {
    if( mask & 0x0001 > 0 ) {strcat( s, names[i] ); if(mask>>1) strcat(s, ", ");}
    i++;
    mask >>=1;
  }
  strcat(s,"\n");
  return s;
} 

static void TaskRAES( void *pvParameters ){
  int8_t state1=1, state2=0; 
  int16_t msgS=0, msgA=LowerPoleDown; 
  int16_t sens=DetailIsReady; // initial state of sensors 
  char buf[256];
  xQueueSendToBack( A, &msgA, portMAX_DELAY );
  for(;;){
    xQueueReceive( S, &msgS, portMAX_DELAY ); 
    sens|=msgS;
//MaskToNames("state of sensors: ",sens,SensorName,buf); console_print( buf );
//sprintf(buf,"state1=%d state2=%d\n", state1, state2); console_print( buf );
    // Generate control according to PN model
    
    if( sens&DetailIsReady &&  sens&ThereIsLeftTurnOfLever && sens&ThereIsLeftTurnOfHead ) {
         sens^=ThereIsLeftTurnOfLever | ThereIsLeftTurnOfHead; msgA=LowerPoleDown; }
    else if( sens&ThereIsRightTurnOfLever &&  sens&ThereIsLeftTurnOfHead ) {
         sens^=ThereIsRightTurnOfLever | ThereIsLeftTurnOfHead; msgA=LowerPoleDown; }
    else if( sens&ThereIsRightTurnOfHead ) {
         sens^=ThereIsRightTurnOfHead; msgA=LowerPoleDown; }

    else if( sens&Claw1IsOpen ) {
         sens^=Claw1IsOpen; msgA=RaisePoleUp;  }
    else if( sens&Claw1Grasped ) {
         sens^=Claw1Grasped; msgA=RaisePoleUp;  }
    else if( sens&Claw2IsOpen ) {
         sens^=Claw2IsOpen; msgA=RaisePoleUp;  }
    else if( sens&Claw2Grasped ) {
         sens^=Claw2Grasped; msgA=RaisePoleUp;  }

    else if( state1==0 && sens&StorageIsReady && sens&ThereIsUpperPositionOfPole ) {
         sens^=StorageIsReady | ThereIsUpperPositionOfPole;   msgA=LowerPoleDown; state1=1; }
    else if( state1==1 && sens&ThereIsUpperPositionOfPole ) {
         sens^=ThereIsUpperPositionOfPole; msgA=RotateLeverToTheLeft|RotateHeadToTheLeft; state1=2; }
    else if( state1==2 && sens&ThereIsUpperPositionOfPole ) {
         sens^=ThereIsUpperPositionOfPole; msgA=RotateHeadToTheRight; state1=3; }
    else if( state1==3 && sens&ThereIsUpperPositionOfPole ) {
         sens^=ThereIsUpperPositionOfPole; msgA=BilletIsDelivered|RotateLeverToTheRight|RotateHeadToTheLeft; state1=4; }
    else if( state1==4 && sens&ThereIsUpperPositionOfPole ) {
         sens^=ThereIsUpperPositionOfPole; msgA=NextToStorage|RotateHeadToTheRight; state1=0; }
    
    else if( state2==0 && sens&ThereIsLowerPositionOfPole ) {
         sens^=ThereIsLowerPositionOfPole; msgA=GraspByClaw1; state2=1; }
    else if( state2==1 && sens&ThereIsLowerPositionOfPole ) {
         sens^=ThereIsLowerPositionOfPole; msgA=GraspByClaw2; state2=2; }
    else if( state2==2 && sens&ThereIsLowerPositionOfPole ) {
         sens^=ThereIsLowerPositionOfPole; msgA=OpenClaw1; state2=3; }
    else if( state2==3 && sens&ThereIsLowerPositionOfPole ) {
         sens^=ThereIsLowerPositionOfPole; msgA=OpenClaw2; state2=0; }
    
    else { MaskToNames("*** error: unrecognized configuration: ",sens,SensorName,buf); 
           console_print( buf ); msgA=0; }

    xQueueSendToBack( A, &msgA, portMAX_DELAY );
  }
}

static void TaskRAP( void *pvParameters ){ // Predefine sensors required to start
  int16_t msgA, msgS;
  char buf[256];
  for(;;){
   xQueueReceive( A, &msgA, portMAX_DELAY );
   //sprintf(buf,"actuators: 0x%08x\n", msgA);
   MaskToNames("plant - actuators in: ",msgA,ActuatorName,buf);
   console_print( buf );

   vTaskDelay( ( rand() % 1000 ) );
   msgS=msgA; // instead of processing all variants

   //sprintf(buf,"sensors: 0x%08x\n", msgS);
   MaskToNames("plant - sensors out: ",msgA,SensorName,buf);
   console_print( buf );
   xQueueSendToBack( S, &msgS, portMAX_DELAY );
  }
}

void MainApp( void ) {
  S = xQueueCreate( 1, sizeof( int16_t ) ); 
  A = xQueueCreate( 1, sizeof( int16_t ) ); 
  xTaskCreate( TaskRAES, "TaskRAES", 1000, ( void * ) 100, 1, NULL );
  xTaskCreate( TaskRAP, " TaskRAES", 1000, ( void * ) 100, 1, NULL );  
  vTaskStartScheduler();
}

// end of RACS @ 2025 daze@acm.org

// Test run (head):

/*

plant - actuators in: LowerPoleDown
plant - sensors out: ThereIsLowerPositionOfPole
plant - actuators in: GraspByClaw1
plant - sensors out: Claw1Grasped
plant - actuators in: RaisePoleUp
plant - sensors out: ThereIsUpperPositionOfPole
plant - actuators in: RotateLeverToTheLeft, RotateHeadToTheLeft
plant - sensors out: ThereIsLeftTurnOfLever, ThereIsLeftTurnOfHead
plant - actuators in: LowerPoleDown
plant - sensors out: ThereIsLowerPositionOfPole
plant - actuators in: GraspByClaw2
plant - sensors out: Claw2Grasped
plant - actuators in: RaisePoleUp
plant - sensors out: ThereIsUpperPositionOfPole
plant - actuators in: RotateHeadToTheRight
plant - sensors out: ThereIsRightTurnOfHead
plant - actuators in: LowerPoleDown
plant - sensors out: ThereIsLowerPositionOfPole
plant - actuators in: OpenClaw1
plant - sensors out: Claw1IsOpen
plant - actuators in: RaisePoleUp
plant - sensors out: ThereIsUpperPositionOfPole
plant - actuators in: BilletIsDelivered, RotateLeverToTheRight, RotateHeadToTheLeft
plant - sensors out: DetailIsReady, ThereIsRightTurnOfLever, ThereIsLeftTurnOfHead
plant - actuators in: LowerPoleDown
plant - sensors out: ThereIsLowerPositionOfPole
plant - actuators in: OpenClaw2
plant - sensors out: Claw2IsOpen
plant - actuators in: RaisePoleUp
plant - sensors out: ThereIsUpperPositionOfPole
plant - actuators in: NextToStorage, RotateHeadToTheRight
plant - sensors out: StorageIsReady, ThereIsRightTurnOfHead
plant - actuators in: LowerPoleDown
plant - sensors out: ThereIsLowerPositionOfPole
plant - actuators in: GraspByClaw1
plant - sensors out: Claw1Grasped
plant - actuators in: RaisePoleUp
plant - sensors out: ThereIsUpperPositionOfPole
plant - actuators in: LowerPoleDown
plant - sensors out: ThereIsLowerPositionOfPole
plant - actuators in: GraspByClaw2
plant - sensors out: Claw2Grasped
plant - actuators in: RaisePoleUp
plant - sensors out: ThereIsUpperPositionOfPole
plant - actuators in: RotateLeverToTheLeft, RotateHeadToTheLeft
plant - sensors out: ThereIsLeftTurnOfLever, ThereIsLeftTurnOfHead
plant - actuators in: LowerPoleDown
plant - sensors out: ThereIsLowerPositionOfPole
plant - actuators in: OpenClaw1
plant - sensors out: Claw1IsOpen
plant - actuators in: RaisePoleUp
...
*/

