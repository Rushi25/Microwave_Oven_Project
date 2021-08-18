/*
 * File:   main.c
 * Author: rushi
 *
 * Created on 28 July, 2021, 11:17 AM
 */

//Including header files
#include <xc.h>
#include "clcd.h"
#include "main.h"
#include "matrix_keypad.h"
#include "timers.h"

#pragma config  WDTE = 0FF // Watchdog timer Enable bit

int operation_mode, reset_flag; //Mode Flags
int min, sec;
int ret, flag = 0, heat_flag = 0; //return flags
char min_array[3], sec_array[3]; //Time Array

static void init_config(void)
{
    /*Initialize CLCD Module*/
    init_clcd();
    /*Initialize MKP Module*/
    init_matrix_keypad();
    //Initialize TIMER2 module
    init_timer2();
    //Initialize Buzzer
    BUZZER_DDR = 0; //OUTPUT
    //Turn off Buzzer
    BUZZER = OFF;
    //Initialize Fan
    FAN_DDR = 0;
    //Turn off Fan
    FAN = OFF;
    PEIE = 1;
    GIE = 1;
}

void main(void)
{
    init_config();
    power_on_screen(); //To display power on message
    clear_display();
    operation_mode = COOKING_MODE_DISPLAY;
    loop();
}

void loop(void)
{
    unsigned char key;
    while(1)
    {
        key = read_matrix_keypad(STATE); //key1 -> 1
        
        if(operation_mode == MICRO_MODE || operation_mode == GRILL_MODE || operation_mode == CONVECTION_MODE)
        {
            ;
        }
        
        //if MKP key1 is pressed
        else if(key == 1)
        {
            //Enable Micro mode of cooking
            operation_mode = MICRO_MODE;
            reset_flag = MICRO_MODE_RESET;
            clear_display();
            clcd_print(" Power = 900W ", LINE2(0));
            __delay_ms(3000); // 3 sec delay
            clear_display();
        }
        
        //If MKP Key2 is pressed
        else if(key == 2)
        {
            //Enable Grill mode of cooking
            operation_mode = GRILL_MODE;
            reset_flag = GRILL_MODE_RESET;
            clear_display();
        }
        
        //If MKP Key3 is pressed
        else if(key == 3)
        {
            //Enable Convection mode of cooking
            operation_mode = CONVECTION_MODE;
            reset_flag = RESET_TEMP;
            clear_display();
        }
        
        //If MKP Key4 is pressed
        else if(key == 4) {
            if(operation_mode == COOKING_MODE_DISPLAY)
            {
                //Enable Start mode
                min = 0;
                sec = 30;
                TMR2ON = ON;
                FAN = ON;
                operation_mode = TIME_DISPLAY;
            }
            else if(operation_mode == TIME_DISPLAY) //For Incrementing sec +30
            {
                sec = sec + 30; // sec -> 60, 90, 120....
                // min++ after 60 sec
                if(sec >= 60) 
                {
                    min++;
                    sec = sec - 60;
                }
            }
            else if(operation_mode == PAUSE) //For Resume Function
            {
                TMR2ON = ON;
                FAN = ON;
                operation_mode = TIME_DISPLAY;
            }
        }
        
        //If MKP Key5 is pressed
        else if(key == 5) // Pause Mode
        {
            operation_mode = PAUSE;
        }
        
        //If MKP Key6 is pressed
        else if(key == 6) // Stop Mode
        {
            operation_mode = STOP;
            clear_display();
        }
        
        operation_call(key);
    }
}

void operation_call(unsigned char key)
{
    switch(operation_mode) 
    {
        case COOKING_MODE_DISPLAY:
            //Display Cooking Modes ie. MICRO, GRILL, CONVENTION, START
            cooking_mode_display();
            break;
            
        case TIME_DISPLAY:
            //Display countdown of TIME 
            time_display();
            break;
            
        case MICRO_MODE:
            //Display set time screen
            set_time(key);
            break;
            
        case GRILL_MODE:
            set_time(key);
            break;
            
        case CONVECTION_MODE:
            if(heat_flag == 0) // For only once execution
            {
                ret = set_temp(key); //Display set temp. screen
                if(ret == FAILURE) //When wrong temp. input
                {
                    flag = 1;
                    reset_flag = RESET_TEMP;
                }
                else if(ret == SUCCESS)
                {
                    TMR2ON = OFF;
                    flag = 1;
                    heat_flag = 1;
                    clear_display();
                    reset_flag = RESET_TIME;
                }
                else
                {
                    flag = 0;
                }
            }
            else
            {
                set_time(key);
                reset_flag = RESET_NOTHING;
            }
            break;
            
        case PAUSE:
            //Turn off timer
            TMR2ON = OFF;
            //Turn off fan
            FAN = OFF;
            break;
            
        case STOP:
            //Turn off timer
            TMR2ON = OFF;
            //Turn off fan
            FAN = OFF;
            //Go back to COOKING_MODE_DISPLAY
            operation_mode = COOKING_MODE_DISPLAY;
            break;
    }
    if(flag == 0)
    {
        reset_flag = RESET_NOTHING;
    }
}

// To Print Power on Message
void power_on_screen(void)
{
    for(int i = 0; i < 16; i++)
    {
        clcd_putch(BLOCK, LINE1(i)); // 0 to 15 for whole line 1
    }
    clcd_print("  Powering ON ", LINE2(0));
    clcd_print(" Microwave Oven ", LINE3(0));
    for(int i = 0; i < 16; i++)
    {
        clcd_putch(BLOCK, LINE4(i)); // 0 to 15 for whole line 4
    }
    // Delay
    __delay_ms(1000); // 3sec
}

void cooking_mode_display(void){
    clcd_print("1.Micro", LINE1(0));
    clcd_print("2.Grill", LINE2(0));
    clcd_print("3.Convection", LINE3(0));
    clcd_print("4.Start", LINE4(0));
}

void clear_display(void){
    clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
}

void set_time(unsigned char key)
{
    static int blink, wait, blink_pos, key_count;
    if(reset_flag >= 0x11) // True only 1st time // 2nd time false
    {
        clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
        key = ALL_RELEASED;
        key_count = 0;
        sec = 0;
        min = 0;
        blink = 0;
        blink_pos = 0;
        clcd_print("SET TIME (MM:SS)", LINE1(0));
        clcd_print("TIME- 00:00", LINE2(0));
        //          012345678910 
        clcd_print("*:CLEAR  #:ENTER", LINE4(0));
    }
    
    //Based on MKP Key press -> set number of min. and sec.
    // Key -> 1 2 3 4 5 6 7 8 9 0
    //   -> ALL_RELEASED , '*', '#'
    if(key != ALL_RELEASED && key != '*' && key != '#')
    {
        //key -> 0 1 2 3 .... 9
        key_count++; // 1 2
        if(key_count <= 2)
        {
            sec = sec * 10 + key; // key = 1 key -> 9
            //sec -> 1 // sec = 1 * 10 + 9 = 19
        }
        else if(key_count > 2 && key_count < 5)
        {
            min = min * 10 + key;
        }
        if(key_count < 2) 
        {
            blink_pos = 0;
        }
        else if(key_count >= 2 && key_count < 4) 
        {
            blink_pos = 1;
        }
    }
    
    
    //To clear sec & min
    else if(key == '*') {
        if(key_count <= 2)
        {
            sec = 0;
            key_count = 0;
            blink_pos = 0;
        }
        else if(key_count > 2 && key_count < 5)
        {
            min = 0;
            key_count = 2;
            blink_pos = 1;
        }
    }
    
    else if(key == '#')
    {
        //Start timer2
        clear_display();
        TMR2ON = 1;
        FAN = ON;
        operation_mode = TIME_DISPLAY;
        
    }
    
    //Print min. & sec. on screen
    //sec = 19 | 21 | 42 | 32
    //sec_array[0] = '1'
    //sec_array[1] = '9'
    sec_array[0] = sec / 10 + '0'; // '1'
    sec_array[1] = sec % 10 + '0'; // '9'
    sec_array[2] = '\0';
    //sec_array[2] = '\0';
    //min -> 34 -> "34"
    min_array[0] = min / 10 + '0'; // 3 + '0' -> '3'
    min_array[1] = min % 10 + '0'; // 4 + '0' -> '4'
    min_array[2] = '\0';
    //min_array[2] = '\0';
       
    if(wait++ == 50) 
    {
        wait = 0;
        blink = !blink;
        clcd_print(min_array, LINE2(6)); // 6 & 7th
        clcd_print(sec_array, LINE2(9)); // 9 & 10th
    }
    if(blink)
    {
        switch(blink_pos)
        {
            case 0: // Blink sec.
                clcd_print("  ", LINE2(9)); // 9 10
                break;
            case 1: // Blink min.
                clcd_print("  ", LINE2(6)); // 6 7
                break;
        }
    }
}

char door_status_check(void) 
{
    // RB3 switch press event as door open
    if(DOOR == OPEN) //RB3 == 0
    {
        BUZZER = ON;
        FAN = OFF;
        TMR2ON = 0;
        clear_display();
        clcd_print("Door Status:OPEN", LINE2(0));
        clcd_print("  Please Close  ", LINE3(0));
        while(DOOR == OPEN)// Block if door is open // RB3 == 0
        {
            ;
        }
        clear_display();
        TMR2ON = 1; //Timer 2 ON
        BUZZER = OFF;
        FAN = ON;
    }
    return CLOSED;
}
void time_display(void)
{
    door_status_check();
    clcd_print(" TIME =  ", LINE1(0));
    //min -> remaining min
    // min -> 22 -> "22"
    min_array[0] = min / 10 + '0'; // 3 + '0' -> '3'
    min_array[1] = min % 10 + '0'; // 4 + '0' -> '4'
    min_array[2] = '\0';
    clcd_print(min_array, LINE1(9));
    clcd_putch(':', LINE1(11));
    
    /*sec_array[0] = sec / 10 + '0'; // '1'
    sec_array[1] = sec % 10 + '0'; // '9'
    sec_array[2] = '\0';
    clcd_print(sec_array, LINE1());
    //sec_array[2] = '\0';
    //min -> 34 -> "34"*/
    
    //sec -> 12
    clcd_putch(sec / 10 + '0', LINE1(12));
    clcd_putch(sec % 10 + '0', LINE1(13));
    
    //To print line 2
    clcd_print(" 4.START/RESUME ", LINE2(0));
    clcd_print(" 5.PAUSE        ", LINE3(0));
    clcd_print(" 6.STOP         ", LINE4(0));
    
    //when time up
    if(min == 0 && sec == 0)
    {
        TMR2ON = 0; //Turning on timer
        FAN = OFF; // Turn OFF FAN
        clear_display();
        clcd_print("  COOKING TIME  ", LINE2(0));
        clcd_print("      IS UP     ", LINE3(0));
        //Turn ON Buzzer
        BUZZER = ON;
        __delay_ms(3000);
        BUZZER = OFF;
        clear_display();
        operation_mode = COOKING_MODE_DISPLAY;
    }
}

char set_temp(unsigned char key) 
{
    static int blink, wait, key_count, temp, door;
    if(reset_flag == RESET_TEMP) // True only 1st time // 2nd time false
    {
        key = ALL_RELEASED;
        key_count = 0;
        blink = 0;
        temp = 0;
        wait = 0;
        clcd_print("SET TEMP. ( ", LINE1(0));
        clcd_putch(DEGREE, LINE1(12));
        clcd_print("C )", LINE1(13));
        clcd_print(" TEMP : 000 ", LINE2(0));
        //          012345678910 
        clcd_print("*:CLEAR  #:ENTER", LINE4(0));
    }
    //Based on MKP Key press -> set temp
    // Key -> 1 2 3 4 5 6 7 8 9 0
    //   -> ALL_RELEASED , '*', '#'
    if(key != ALL_RELEASED && key != '*' && key != '#')
    {
        //key -> 0 1 2 3 .... 9
        key_count++; // 1 2
        if(key_count <= 3)
        {
            temp = temp * 10 + key; // key = 1 key -> 9
            //temp -> 1 // temp = 1 * 10 + 9 = 19
        }
    }
    //To clear temp
    else if(key == '*')
    {
        key_count = 0;
        temp = 0;
    }
    
    else if(key == '#')
    {
        // Valid temp range upto 250
        if(temp >= 250) 
        {
            clear_display();
            clcd_print("  Invalid Temp. ", LINE2(0));
            BUZZER = ON;
            __delay_ms(2000);
            BUZZER = OFF;
            clear_display();
            return FAILURE;
        }
        else
        {
            clear_display();
            clcd_print("  Pre-Heating  ", LINE1(0));
            sec = 180;
            TMR2ON = ON;
            clcd_print(" Time Rem.=", LINE3(0));
                      //0123456789
            while(sec)// sec = 0 after 3 min
            {
                door = door_status_check();
                if (door == CLOSED)
                {
                    clcd_print("  Pre-Heating  ", LINE1(0));
                    clcd_print(" Time Rem.=", LINE3(0));
                    FAN = OFF;
                }
                clcd_putch((sec/100 + '0'), LINE3(11));
                clcd_putch(((sec/10) % 10 + '0'), LINE3(12));
                clcd_putch((sec % 10 + '0'), LINE3(13));
            }
            return SUCCESS;
        }
    }
    if(wait++ == 50) 
    {
        wait = 0;
        blink = !blink;
        // Temp -> 0 -> 123
        clcd_putch((temp/100 + '0'), LINE2(8)); // 123/100 -> 1 + '0'
        clcd_putch(((temp/10) % 10 + '0'), LINE2(9)); // 123/10 -> 12%10 -> 2 + '0'
        clcd_putch((temp % 10 + '0'), LINE2(10)); // 123%10 -> 3 + '0'
    }
    if(blink)
    {
        clcd_print("   ", LINE2(8)); // 9 10
    }
    return 0x0F;
}