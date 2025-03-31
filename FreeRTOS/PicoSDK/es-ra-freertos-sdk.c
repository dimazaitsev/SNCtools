// RACPico: Robotic Arm Control, FreeRTOS, Rasberry Pi Pico
//
// The automated manufacture shop composition of a robotic arm, storage (palette), and rotary table is specified in:
// Zaitsev D.A. Petri Nets and Modelling of Systems: textbook for laboratory training. Odessa: ONAT, 2007, 42 p. In Ukr. / Eng.
// https://dimazaitsev.github.io/pdf/pnms-en.pdf
// The control algorithm is given by a Sleptsov Net (https://dimazaitsev.github.io/snc.html)
// We use LCD to indicate actuators and sensors and keypad to input sensor. 
// Connection: Keypad GPIO 13, 12, 11, 10, 9, 8, 7, 6; LCD GPIO 4 sda, 5 scl and 3.3v, gnd (see photo)
//
// Install and run software in FreeRTOS RP Pico port:
// https://github.com/aws-iot-builder-tools/freertos-pi-pico
// 1) save this file as main.c to app
// 2) add the line to CMakeLists.txt: target_link_libraries(app pico_stdlib hardware_i2c)
// 3) cd build; cmake ..; make
// 4) upload app.uf2 to pico
// 5) Input sensors corresponding to actuators completed by #
// 6) Check the model trace appended at the bottom
//
// Jump to the application directory cd /home/sysprog/pisdk/freertos-pi-pico/app/build
//
// Please excuse for a clumsy insertion of code (to have a single .c) for:
// keypad from https://github.com/hhoswaldo/pico-keypad4x4
// LCD from https://github.com/raspberrypi/pico-examples/tree/master/i2c/lcd_1602_i2c
//
// Sleptsov steers !
// https://youtu.be/7HPnhaNGyqo

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>

#include "pico/stdlib.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// *** Keypad code from https://github.com/hhoswaldo/pico-keypad4x4

//byte rowPin[4] = {13, 12, 11, 10}; // Arduino
//byte colPin[4] = { 9,  8,  7,  6};
//
//char keyStrings[4][4] = {
//  {'1', '2', '3', 'A'},
//  {'4', '5', '6', 'B'},
//  {'7', '8', '9', 'C'},
//  {'*', '0', '#', 'D'}
//};

uint columns[4] = { 9,  8,  7,  6 };
uint rows[4] = { 13, 12, 11, 10 };
char matrix[16] = {
    '1', '2' , '3', 'A',
    '4', '5' , '6', 'B',
    '7', '8' , '9', 'C',
    '*', '0' , '#', 'D'
};

#define GPIO_INPUT false
#define GPIO_OUTPUT true

uint _columns[4];
uint _rows[4];
char _matrix_values[16];

uint all_columns_mask = 0x0;
uint column_mask[4];

/**
 * @brief Set up the keypad
 *
 * @param columns Pins connected to keypad columns
 * @param rows Pins connected to keypad rows
 * @param matrix_values values assigned to each key
 */
void pico_keypad_init(uint columns[4], uint rows[4], char matrix_values[16]) {

    for (int i = 0; i < 16; i++) {
        _matrix_values[i] = matrix_values[i];
    }

    for (int i = 0; i < 4; i++) {

        _columns[i] = columns[i];
        _rows[i] = rows[i];

        gpio_init(_columns[i]);
        gpio_init(_rows[i]);

        gpio_set_dir(_columns[i], GPIO_INPUT);
        gpio_set_dir(_rows[i], GPIO_OUTPUT);

        gpio_put(_rows[i], 1);

        all_columns_mask = all_columns_mask + (1 << _columns[i]);
        column_mask[i] = 1 << _columns[i];
    }
}

/**
 * @brief Scan and get the pressed key.
 *
 * This routine returns the first key found to be pressed
 * during the scan.
 */
char pico_keypad_get_key(void) {
    int row;
    uint32_t cols;
    bool pressed = false;

    cols = gpio_get_all();
    cols = cols & all_columns_mask;

    if (cols == 0x0) {
        return 0;
    }

    for (int j = 0; j < 4; j++) {
        gpio_put(_rows[j], 0);
    }

    for (row = 0; row < 4; row++) {

        gpio_put(_rows[row], 1);

        busy_wait_us(10000);

        cols = gpio_get_all();
        gpio_put(_rows[row], 0);
        cols = cols & all_columns_mask;
        if (cols != 0x0) {
            break;
        }
    }

    for (int i = 0; i < 4; i++) {
        gpio_put(_rows[i], 1);
    }

    if (cols == column_mask[0]) {
        return (char)_matrix_values[row * 4 + 0];
    }
    else if (cols == column_mask[1]) {
        return (char)_matrix_values[row * 4 + 1];
    }
    if (cols == column_mask[2]) {
        return (char)_matrix_values[row * 4 + 2];
    }
    else if (cols == column_mask[3]) {
        return (char)_matrix_values[row * 4 + 3];
    }
    else {
        return 0;
    }
}

// *** end of Keypad code insertion


// *** LCD code from https://github.com/raspberrypi/pico-examples/tree/master/i2c/lcd_1602_i2c

#include "/home/sysprog/pisdk/pico-sdk/src/rp2_common/hardware_i2c/include/hardware/i2c.h"
#include "pico/binary_info.h"

/* Example code to drive a 16x2 LCD panel via a I2C bridge chip (e.g. PCF8574)

   NOTE: The panel must be capable of being driven at 3.3v NOT 5v. The Pico
   GPIO (and therefore I2C) cannot be used at 5v.

   You will need to use a level shifter on the I2C lines if you want to run the
   board at 5v.

   Connections on Raspberry Pi Pico board, other boards may vary.

   GPIO 4 (pin 6)-> SDA on LCD bridge board
   GPIO 5 (pin 7)-> SCL on LCD bridge board
   3.3v (pin 36) -> VCC on LCD bridge board
   GND (pin 38)  -> GND on LCD bridge board
*/
// commands
const int LCD_CLEARDISPLAY = 0x01;
const int LCD_RETURNHOME = 0x02;
const int LCD_ENTRYMODESET = 0x04;
const int LCD_DISPLAYCONTROL = 0x08;
const int LCD_CURSORSHIFT = 0x10;
const int LCD_FUNCTIONSET = 0x20;
const int LCD_SETCGRAMADDR = 0x40;
const int LCD_SETDDRAMADDR = 0x80;

// flags for display entry mode
const int LCD_ENTRYSHIFTINCREMENT = 0x01;
const int LCD_ENTRYLEFT = 0x02;

// flags for display and cursor control
const int LCD_BLINKON = 0x01;
const int LCD_CURSORON = 0x02;
const int LCD_DISPLAYON = 0x04;

// flags for display and cursor shift
const int LCD_MOVERIGHT = 0x04;
const int LCD_DISPLAYMOVE = 0x08;

// flags for function set
const int LCD_5x10DOTS = 0x04;
const int LCD_2LINE = 0x08;
const int LCD_8BITMODE = 0x10;

// flag for backlight control
const int LCD_BACKLIGHT = 0x08;

const int LCD_ENABLE_BIT = 0x04;

// By default these LCD display drivers are on bus address 0x27
static int addr = 0x27;

// Modes for lcd_send_byte
#define LCD_CHARACTER  1
#define LCD_COMMAND    0

#define MAX_LINES      2
#define MAX_CHARS      16

/* Quick helper function for single byte transfers */
void i2c_write_byte(uint8_t val) {
#ifdef i2c_default
    i2c_write_blocking(i2c_default, addr, &val, 1, false);
#endif
}

void lcd_toggle_enable(uint8_t val) {
    // Toggle enable pin on LCD display
    // We cannot do this too quickly or things don't work
#define DELAY_US 600
    sleep_us(DELAY_US);
    i2c_write_byte(val | LCD_ENABLE_BIT);
    sleep_us(DELAY_US);
    i2c_write_byte(val & ~LCD_ENABLE_BIT);
    sleep_us(DELAY_US);
}

// The display is sent a byte as two separate nibble transfers
void lcd_send_byte(uint8_t val, int mode) {
    uint8_t high = mode | (val & 0xF0) | LCD_BACKLIGHT;
    uint8_t low = mode | ((val << 4) & 0xF0) | LCD_BACKLIGHT;

    i2c_write_byte(high);
    lcd_toggle_enable(high);
    i2c_write_byte(low);
    lcd_toggle_enable(low);
}

void lcd_clear(void) {
    lcd_send_byte(LCD_CLEARDISPLAY, LCD_COMMAND);
}

// go to location on LCD
void lcd_set_cursor(int line, int position) {
    int val = (line == 0) ? 0x80 + position : 0xC0 + position;
    lcd_send_byte(val, LCD_COMMAND);
}

static inline void lcd_char(char val) {
    lcd_send_byte(val, LCD_CHARACTER);
}

void lcd_string(const char *s) {
    while (*s) {
        lcd_char(*s++);
    }
}

void lcd_init() {
    lcd_send_byte(0x03, LCD_COMMAND);
    lcd_send_byte(0x03, LCD_COMMAND);
    lcd_send_byte(0x03, LCD_COMMAND);
    lcd_send_byte(0x02, LCD_COMMAND);

    lcd_send_byte(LCD_ENTRYMODESET | LCD_ENTRYLEFT, LCD_COMMAND);
    lcd_send_byte(LCD_FUNCTIONSET | LCD_2LINE, LCD_COMMAND);
    lcd_send_byte(LCD_DISPLAYCONTROL | LCD_DISPLAYON, LCD_COMMAND);
    lcd_clear();
}

// *** end of LCD code insertion


QueueHandle_t S, A; // modeling Sensors and Actuators

// names of sensors; masks of sensors; numbers of sensors for input

#define NUM_S                           12
#define StorageIsReady                  0x0001  /* 0 */
#define DetailIsReady                   0x0002  /* 1 */
#define ThereIsLeftTurnOfLever          0x0004  /* 2 */
#define ThereIsRightTurnOfLever         0x0008  /* 3 */
#define ThereIsLeftTurnOfHead           0x0010  /* 4 */
#define ThereIsRightTurnOfHead          0x0020  /* 5 */
#define ThereIsLowerPositionOfPole      0x0040  /* 6 */
#define ThereIsUpperPositionOfPole      0x0080  /* 7 */
#define Claw1Grasped                    0x0100  /* 8 */
#define Claw2Grasped                    0x0200  /* 9 */
#define Claw1IsOpen                     0x0400  /* A */
#define Claw2IsOpen                     0x0800  /* B */

// names of actuators; masks of actuators; numbers of actuators for output

#define NUM_A                           12
#define NextToStorage                   0x0001  /* 0 */
#define BilletIsDelivered               0x0002  /* 1 */
#define RotateLeverToTheLeft            0x0004  /* 2 */
#define RotateLeverToTheRight           0x0008  /* 3 */
#define RotateHeadToTheLeft             0x0010  /* 4 */
#define RotateHeadToTheRight            0x0020  /* 5 */
#define LowerPoleDown                   0x0040  /* 6 */
#define RaisePoleUp                     0x0080  /* 7 */
#define GraspByClaw1                    0x0100  /* 8 */
#define GraspByClaw2                    0x0200  /* 9 */
#define OpenClaw1                       0x0400  /* A */
#define OpenClaw2                       0x0800  /* B */

int key_to_num(char ch) {
        if (ch >= '0' && ch <= '9')
        return ch - '0';
    if (ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;
    return -1;
}

int num_to_mask(int a) {
        int i,aa=0x0001;
        if(a<0 && a>=NUM_A) return 0x0000;
        for(i=0;i<a;i++) aa<<=1;
        return aa;
}

static void TaskRAES( void *pvParameters ){
  int8_t state1=1, state2=0;
  int16_t msgS=0, msgA=LowerPoleDown;
  int16_t sens=DetailIsReady; // state of sensors

  xQueueSendToBack( A, &msgA, portMAX_DELAY );
  for(;;){
    xQueueReceive( S, &msgS, portMAX_DELAY );
    sens|=msgS;

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
    else msgA=0;

    xQueueSendToBack( A, &msgA, portMAX_DELAY );
  }
}

static void TaskRAP( void *pvParameters ){ // Predefine sensors required to start
  int16_t msgA, msgS;
  int16_t actu=0; // state of actuators
  char key, prevkey, buf[10];
  int16_t p, aa, a;

  for(;;){
   xQueueReceive( A, &msgA, portMAX_DELAY );

// gpio_put(PICO_DEFAULT_LED_PIN, 1); // for debugging

//   actu|=msgA;
   msgS=0;

   // write received actuators on LCD: "actuators started:"
   lcd_clear();
   p=0;
   lcd_set_cursor(0,p);
   lcd_string("actu: ");
   p+=6;
   aa=msgA;
   for(a=0;a<NUM_A;a++) {
         if(aa&0x0001){
           sprintf(buf,"%1X,",a);
           lcd_string(buf);
           p+=2; 
     }
     aa>>=1;
   }

   // input and write on LCD sensor(s) - only correct, util # is pressed: "input sensor(s): "
   p=0;
   lcd_set_cursor(1,p);
   lcd_string("sens: ");
   p+=6;
   prevkey='\0';
   while((key = pico_keypad_get_key())!='#'){
   	    if(key!=prevkey) {
         sprintf(buf,"%c,",key);
         lcd_string(buf);
         p+=2;
         a=key_to_num(key);
         aa=num_to_mask(a);     
         msgS|=aa;
         prevkey=key;
    }
   }
   while((key = pico_keypad_get_key())!='#');

//gpio_put(PICO_DEFAULT_LED_PIN, 0); // for debugging
//vTaskDelay(1000);

   xQueueSendToBack( S, &msgS, portMAX_DELAY );
  }
}

void main( void ) {
//  gpio_init(PICO_DEFAULT_LED_PIN); // for debugging
//  gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

  // keypad
  stdio_init_all();
  pico_keypad_init(columns, rows, matrix);

  // LCD
  // This example will use I2C0 on the default SDA and SCL pins (4, 5 on a Pico)
  i2c_init(i2c_default, 100 * 1000);
  gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
  gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
  gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
  gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
  // Make the I2C pins available to picotool
  bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));
  lcd_init();

  S = xQueueCreate( 1, sizeof( int16_t ) );
  A = xQueueCreate( 1, sizeof( int16_t ) );
  xTaskCreate( TaskRAES, "TaskRAES", 1000, ( void * ) 100, 1, NULL );
  xTaskCreate( TaskRAP, " TaskRAES", 1000, ( void * ) 100, 1, NULL );
  vTaskStartScheduler();
}

// end of RACPico @ 2025 daze@acm.org

/* correct trace:
actu: 6, (LowerPoleDown)
sens: 6# (ThereIsLowerPositionOfPole)
actu: 8, (GraspByClaw1)
sens: 8,# (Claw1Grasped)
actu: 7, (RaisePoleUp)
sens: 7,# (ThereIsUpperPositionOfPole)
actu: 2,4 (RotateLeverToTheLeft, RotateHeadToTheLeft
sens: 2,4,# (ThereIsLeftTurnOfLever, ThereIsLeftTurnOfHead
actu: 6, (LowerPoleDown)
sens: 6,# (ThereIsLowerPositionOfPole)
actu: 9, (GraspByClaw2)
sens: 9,# (Claw2Grasped)
actu: 7, (RaisePoleUp)
sens: 7,# (ThereIsUpperPositionOfPole)
actu: 5, (RotateHeadToTheRight)
sens: 5,# (ThereIsRightTurnOfHead)
actu: 6, (LowerPoleDown)
sens: 6,# (ThereIsLowerPositionOfPole)
actu: A, (OpenClaw1)
sens: A,# (Claw1IsOpen)
actu: 7, (RaisePoleUp)
sens: 7,# (ThereIsUpperPositionOfPole)
actu: 1,3,4  (BilletIsDelivered, RotateLeverToTheRight, RotateHeadToTheLeft)
sens: 1,3,4,# (DetailIsReady, ThereIsRightTurnOfLever, ThereIsLeftTurnOfHead)
actu: 6, (LowerPoleDown)
sens: 6,# (ThereIsLowerPositionOfPole)
actu: B, (OpenClaw2)
sens: B,# (Claw2IsOpen)
actu: 7, (RaisePoleUp)
sens: 7,# (ThereIsUpperPositionOfPole)
actu: 0,5, (NextToStorage, RotateHeadToTheRight)
sens: 0,5,# (StorageIsReady, ThereIsRightTurnOfHead)

actu: 6, (LowerPoleDown)
sens: 6# (ThereIsLowerPositionOfPole)
...
*/
