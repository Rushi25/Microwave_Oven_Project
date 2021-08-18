/* 
 * File:   main.h
 * Author: rushi
 *
 * Created on 28 July, 2021, 11:30 AM
 */

#ifndef MAIN_H
#define	MAIN_H

#define COOKING_MODE_DISPLAY    0x01
#define MICRO_MODE              0x02
#define GRILL_MODE              0x03
#define CONVECTION_MODE         0x04
#define START_MODE              0x05
#define TIME_DISPLAY            0x06
#define STOP                    0x07
#define PAUSE                   0x08

#define MICRO_MODE_RESET        0x11
#define GRILL_MODE_RESET        0x12
#define RESET_TEMP              0x13
#define RESET_TIME              0x14
#define RESET_NOTHING           0x0F

#define BUZZER                  RC1
#define BUZZER_DDR              TRISC1
#define FAN                     RC2
#define FAN_DDR                 TRISC2
#define ON                      1
#define OFF                     0
#define DOOR                    RB3
#define OPEN                    0
#define CLOSED                  1
#define FAILURE                 1
#define SUCCESS                 0

void loop(void);
void power_on_screen(void);
void cooking_mode_display(void);
void clear_display(void);
void operation_call(unsigned char key);
void set_time(unsigned char key);
char set_temp(unsigned char key);
void time_display(void);

#endif	/* MAIN_H */

