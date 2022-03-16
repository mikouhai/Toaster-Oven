// **** Include libraries here ****
// Standard libraries
#include <stdio.h>
#include <GenericTypeDefs.h>

//CSE13E Support Library
#include "BOARD.h"

// Microchip libraries
#include <xc.h>
#include <sys/attribs.h>
#include "Oled.h"
#include "OledDriver.h"
#include "Ascii.h"
#include "Adc.h"
#include "Buttons.h"
#include <string.h>
#include "Leds.h"

// **** Set any macros or preprocessor directives here ****
// Set a macro for resetting the timer, makes the code a little clearer.
#define TIMER_2HZ_RESET() (TMR1 = 0)


// **** Set any local typedefs here ****

typedef enum {
    SETUP, SELECTOR_CHANGE_PENDING, COOKING, RESET_PENDING
} OvenState;

typedef enum {
    TIME, TEMPERATURE
} SELECT;

typedef enum {
    Bake, Broil, Toast
} MODE;

typedef struct {
    OvenState state;
    MODE CookingMode;
    SELECT Select;
    uint16_t TEMPERATURE;
    uint16_t StartCookingTime;
    uint16_t ButtonTimeDiff;
    uint16_t ButtonPressEnd;
    uint16_t ButtonPressStart;
    //add more members to this struct
} OvenData;

struct Timer {
    uint8_t event;
    uint8_t event1;
    uint8_t event2;
    uint8_t event4_1;
    uint8_t event4_2;
    uint8_t event4_3;
};

struct Cook {
    uint8_t event;
};
struct Cook OvenCook;
struct Timer Time;
struct Timer Time = {
    FALSE, FALSE, FALSE, FALSE, FALSE
};
struct Cook OvenCook = {
    FALSE
};
// **** Declare any datatypes here ****
OvenData oven;
#define LEDS 0xFF
#define LEDs7_8 0xFE
#define LEDs6_8 0xFC
#define LEDs5_8 0xF8
#define LEDs4_8 0xF0
#define LEDs3_8 0xE0
#define LEDs2_8 0xC0
#define LEDs1_8 0x80
#define LEDSOFF 0x00
// **** Define any module-level, global, or external variables here ****
char *CookMode[] = {
    "Bake", "Broil", "Toast"
};
char *point[] = {
    " ", ">"
};
int Holder = 0;
int LedAmount = 8;
int Led1_8 = 1 / 8;
int Led2_8 = 2 / 8;
int Led3_8 = 3 / 8;
int Led4_8 = 4 / 8;
int Led5_8 = 5 / 8;
int Led6_8 = 6 / 8;
int Led7_8 = 7 / 8;
int TickCount = 0;
int Counter = 0;
int StopWatch = 0;
int FHz = 5;
int ButtonHold = 9;
int EndOfList = 2;
int StartOfList = -1;
int tim = 60; // time to divide for minutes and seconds
int minutes = 0;
int seconds = 0;
char *OVEN_TOP = OVEN_TOP_OFF;
char *OVEN_BOTTOM = OVEN_BOTTOM_OFF;
int OvenTemp = 300; // the values to change adc to temp
int SWAPAROO = 0; //0 place holder
char *TimePoint = 0;
char *TempPoint = 0;

// **** Put any helper functions here ****

/*This function will update your OLED to reflect the state .*/
void updateOvenOLED(OvenData ovenData) {
    //update OLED here

    OledClear(0);
    char OledString[200];
    sprintf(OledString, "|%s%s%s%s%s|  Mode: %s\n", OVEN_TOP, OVEN_TOP, OVEN_TOP, OVEN_TOP, OVEN_TOP, CookMode[oven.CookingMode]);
    sprintf(OledString + strlen(OledString), "|     | %sTime: %d:%.2d\n", TimePoint, minutes, seconds);
    if (CookMode[oven.CookingMode] == "Toast") { // only toast does not track temp cause its too cool
        sprintf(OledString + strlen(OledString), "|-----|\n");
    } else {
        sprintf(OledString + strlen(OledString), "|-----| %sTemp: %3u%sF\n", TempPoint, oven.TEMPERATURE, DEGREE_SYMBOL);
    }
    sprintf(OledString + strlen(OledString), "|%s%s%s%s%s|", OVEN_BOTTOM, OVEN_BOTTOM, OVEN_BOTTOM, OVEN_BOTTOM, OVEN_BOTTOM);
    OledClear(0);
    OledDrawString(OledString);
    OledUpdate();
}

/*This function will execute your state machine.  
 * It should ONLY run if an event flag has been set.*/
void runOvenSM(void) {
    //write your SM logic here.
    switch (oven.state) {
        case SETUP:
            oven.state = SETUP;
            OVEN_TOP = OVEN_TOP_OFF;
            OVEN_BOTTOM = OVEN_BOTTOM_OFF;
            LEDS_SET(LEDSOFF);
            updateOvenOLED(oven);
            break;
        case SELECTOR_CHANGE_PENDING:
            if ((oven.Select == TIME) && (CookMode[oven.CookingMode] != "Toast")) {
                oven.Select = TEMPERATURE;
            } else {
                oven.Select = TIME;
            }
            if (CookMode[oven.CookingMode] == "Toast") {
                oven.Select = TIME;
            }
            oven.state = SETUP;
            oven.ButtonTimeDiff = SWAPAROO;
            TimePoint = point[oven.Select == TIME];
            TempPoint = point[oven.Select == TEMPERATURE];
            updateOvenOLED(oven);
            break;
        case COOKING:
            if (CookMode[oven.CookingMode] == "Bake") {
                OVEN_TOP = OVEN_TOP_ON;
                OVEN_BOTTOM = OVEN_BOTTOM_ON;
            }
            if (CookMode[oven.CookingMode] == "Toast") {
                OVEN_BOTTOM = OVEN_BOTTOM_ON;
                OVEN_TOP = OVEN_TOP_OFF;
            }
            if (CookMode[oven.CookingMode] == "Broil") {
                OVEN_TOP = OVEN_TOP_ON;
                OVEN_BOTTOM = OVEN_BOTTOM_OFF;
            }
            Led7_8 = (oven.StartCookingTime * 7) / 8;
            Led6_8 = (oven.StartCookingTime * 6) / 8;
            Led5_8 = (oven.StartCookingTime * 5) / 8;
            Led4_8 = (oven.StartCookingTime * 4) / 8;
            Led3_8 = (oven.StartCookingTime * 3) / 8;
            Led2_8 = (oven.StartCookingTime * 2) / 8;
            Led1_8 = (oven.StartCookingTime * 1) / 8;
            updateOvenOLED(oven);
            break;
        case RESET_PENDING:
            LEDS_SET(LEDSOFF);
            Counter = SWAPAROO;
            minutes = oven.StartCookingTime / tim;
            seconds = oven.StartCookingTime % tim;
            oven.state = SETUP;
            oven.ButtonTimeDiff = SWAPAROO;
            break;
    }
}

int main() {
    BOARD_Init();
    LEDS_INIT();
    //initalize timers and timer ISRs:
    // <editor-fold defaultstate="collapsed" desc="TIMER SETUP">

    // Configure Timer 2 using PBCLK as input. We configure it using a 1:16 prescalar, so each timer
    // tick is actually at F_PB / 16 Hz, so setting PR2 to F_PB / 16 / 100 yields a .01s timer.

    T2CON = 0; // everything should be off
    T2CONbits.TCKPS = 0b100; // 1:16 prescaler
    PR2 = BOARD_GetPBClock() / 16 / 100; // interrupt at .5s intervals
    T2CONbits.ON = 1; // turn the timer on

    // Set up the timer interrupt with a priority of 4.
    IFS0bits.T2IF = 0; //clear the interrupt flag before configuring
    IPC2bits.T2IP = 4; // priority of  4
    IPC2bits.T2IS = 0; // subpriority of 0 arbitrarily 
    IEC0bits.T2IE = 1; // turn the interrupt on

    // Configure Timer 3 using PBCLK as input. We configure it using a 1:256 prescaler, so each timer
    // tick is actually at F_PB / 256 Hz, so setting PR3 to F_PB / 256 / 5 yields a .2s timer.

    T3CON = 0; // everything should be off
    T3CONbits.TCKPS = 0b111; // 1:256 prescaler
    PR3 = BOARD_GetPBClock() / 256 / 5; // interrupt at .5s intervals
    T3CONbits.ON = 1; // turn the timer on

    // Set up the timer interrupt with a priority of 4.
    IFS0bits.T3IF = 0; //clear the interrupt flag before configuring
    IPC3bits.T3IP = 4; // priority of  4
    IPC3bits.T3IS = 0; // subpriority of 0 arbitrarily 
    IEC0bits.T3IE = 1; // turn the interrupt on;

    // </editor-fold>

    printf("Welcome to rhuang43's Lab07 (Toaster Oven).  Compiled on %s %s.", __TIME__, __DATE__);

    //initialize state machine (and anything else you need to init) here
    OledInit();
    ButtonsInit();
    AdcInit();
    oven.TEMPERATURE = OvenTemp;
    oven.state = SETUP;
    oven.Select = TIME;
    TimePoint = point[1];
    TempPoint = point[0];

    while (1) {
        runOvenSM();
        updateOvenOLED(oven);
        // Add main loop code here:
        // check for events
        // on event, run runOvenSM()
        // clear event flags
    };
}

/*The 5hz timer is used to update the free-running timer and to generate TIMER_TICK events*/
void __ISR(_TIMER_3_VECTOR, ipl4auto) TimerInterrupt5Hz(void) {
    // Clear the interrupt flag.
    IFS0CLR = 1 << 12;

    //add event-checking code here
    StopWatch++;
    if (Time.event == TRUE) {
        oven.ButtonPressStart = StopWatch;
        Time.event = FALSE;
    }
    if (Time.event1 == TRUE) {
        oven.ButtonPressEnd = StopWatch;
        Time.event1 = FALSE;
    }
    if (Time.event4_1 == TRUE) {
        oven.ButtonPressStart = StopWatch;
        Time.event4_1 = FALSE;
    }
    if (Time.event4_2 == TRUE) {
        oven.ButtonPressEnd = StopWatch;
        Time.event4_2 = FALSE;
    }

    oven.ButtonTimeDiff = (oven.ButtonPressEnd - oven.ButtonPressStart);

    if ((oven.ButtonTimeDiff > ButtonHold) && (Time.event4_3 == TRUE)) {
        Time.event4_3 = FALSE;
        oven.state = RESET_PENDING;
    } else if ((oven.ButtonTimeDiff > ButtonHold) && (Time.event2 == TRUE)) { //since it counts 5 times a second 
        oven.state = SELECTOR_CHANGE_PENDING;
        Time.event2 = FALSE;
    }

    if ((oven.ButtonTimeDiff < ButtonHold) && (Time.event2 == TRUE)) {
        if (oven.CookingMode == EndOfList) {
            oven.CookingMode = StartOfList;
        }
        oven.CookingMode++;
        Time.event2 = FALSE;
        updateOvenOLED(oven);
    }

    if (Counter != SWAPAROO) {
        TickCount++;
        if (TickCount == FHz) {
            Counter--;
            if (Counter == Led7_8) {
                LEDS_SET(LEDs7_8);
            }
            if (Counter == Led6_8) {
                LEDS_SET(LEDs6_8);
            }
            if (Counter == Led5_8) {
                LEDS_SET(LEDs5_8);
            }
            if (Counter == Led4_8) {
                LEDS_SET(LEDs4_8);
            }
            if (Counter == Led3_8) {
                LEDS_SET(LEDs3_8);
            }
            if (Counter == Led2_8) {
                LEDS_SET(LEDs2_8);
            }
            if (Counter == Led1_8) {
                LEDS_SET(LEDs1_8);
            } 
            minutes = Counter / tim;
            seconds = Counter % tim;
            updateOvenOLED(oven);
            TickCount = SWAPAROO;
        }
        if (Counter == SWAPAROO) {
            oven.state = SETUP;
            LEDS_SET(LEDSOFF);
        }
    }
}

/*The 100hz timer is used to check for button and ADC events*/
void __ISR(_TIMER_2_VECTOR, ipl4auto) TimerInterrupt100Hz(void) {
    // Clear the interrupt flag.
    IFS0CLR = 1 << 8;

    //add event-checking code here
    uint8_t bEvent = ButtonsCheckEvents();
    if (bEvent & BUTTON_EVENT_3DOWN) { // if button 3 is pressed down 
        Time.event = TRUE;
        Time.event2 = TRUE;
    }
    if (bEvent & BUTTON_EVENT_3UP) {
        Time.event1 = TRUE;
    }

    if (bEvent & BUTTON_EVENT_4DOWN) {
        oven.state = COOKING; //start oven
        LEDS_SET(LEDS);
        Time.event4_1 = TRUE;
        Counter = oven.StartCookingTime;
    }
    if (bEvent & BUTTON_EVENT_4UP) {
        Time.event4_2 = TRUE;
        Time.event4_3 = TRUE;
    }
    if (oven.Select == TEMPERATURE) {
        if (AdcChanged() == TRUE) {
            oven.TEMPERATURE = AdcRead();
            oven.TEMPERATURE = oven.TEMPERATURE >> EndOfList; // using end of list to only use the top 8 numbers since end of list is 2
            oven.TEMPERATURE += OvenTemp;
        }
    }
    if (oven.Select == TIME) {
        if (AdcChanged() == TRUE) { //change in time for potentiometer 
            oven.StartCookingTime = AdcRead();
            oven.StartCookingTime = oven.StartCookingTime >> EndOfList; //although end of list is 2 for the list length we are using it to right rotate twice to avoid magic numbers
            oven.StartCookingTime++;
            minutes = oven.StartCookingTime / tim;
            seconds = oven.StartCookingTime % tim;
        }
    }
}
