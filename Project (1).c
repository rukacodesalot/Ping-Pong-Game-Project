// =============================================================================
// ECNG 1502 - PING PONG GAME CONTROLLER
// PIC16F877A @ 4MHz External Crystal
// Buttons: RC0 (P1), RC1 (P2) - External 10kΩ pull-up resistors (active LOW)
// Potentiometer: RA0 | LEDs: RB0-RB6 | 7-seg: PORTD
// Hardware: P1 is at RB6 (pos 6), P2 is at RB0 (pos 0)
// Serve rule: whoever SCORES the point serves next
// =============================================================================

#include <xc.h>
#include <stdio.h>

#pragma config FOSC  = XT
#pragma config WDTE  = OFF
#pragma config PWRTE = ON
#pragma config BOREN = OFF
#pragma config LVP   = OFF
#pragma config CPD   = OFF
#pragma config WRT   = OFF
#pragma config CP    = OFF

#define _XTAL_FREQ 4000000UL

const unsigned char seg[10] = {
    0xFC, 0x60, 0xDA, 0xF2, 0x66, 0xB6, 0xBE, 0xE0, 0xFE, 0xF6
};

// ---------------------------------------------------------------------------
// Global Game Variables
// ---------------------------------------------------------------------------
unsigned char score_p1      = 0;
unsigned char score_p2      = 0;
unsigned int  base_delay    = 200;
signed char   ball_position = 6;    // P1 starts at pos 6 (RB6)
unsigned char current_level = 0;

// ---------------------------------------------------------------------------
// Game States
// ---------------------------------------------------------------------------
// 1 = Serve: ball at pos 6 (RB6), P1 presses RC0 to launch
// 2 = Serve: ball at pos 0 (RB0), P2 presses RC1 to launch
// 3 = Ball moving LEFT  (pos 5→1), only P2 pressing early is a foul
// 4 = Ball moving RIGHT (pos 1→5), only P1 pressing early is a foul
// 5 = Ball arrived at pos 0 (RB0), P2 must HIT within timeout or miss
// 6 = Ball arrived at pos 6 (RB6), P1 must HIT within timeout or miss
unsigned char game_state = 1;

// ---------------------------------------------------------------------------
// UART
// ---------------------------------------------------------------------------
void UART_Init(void) {
    TRISCbits.TRISC6 = 0;
    TRISCbits.TRISC7 = 1;
    SPBRG = 25;
    TXSTA = 0x24;
    RCSTA = 0x90;
}

void putch(char data) {
    while (!TXSTAbits.TRMT);
    TXREG = data;
}

// ---------------------------------------------------------------------------
// ADC
// ---------------------------------------------------------------------------
void Update_Speed(void) {
    __delay_us(20);
    ADCON0bits.GO = 1;
    while (ADCON0bits.GO);
    unsigned int adc = (unsigned int)((ADRESH << 8) | ADRESL);

    unsigned char lvl;
    if      (adc < 205) { lvl = 1; base_delay = 450; }
    else if (adc < 410) { lvl = 2; base_delay = 350; }
    else if (adc < 614) { lvl = 3; base_delay = 250; }
    else if (adc < 819) { lvl = 4; base_delay = 160; }
    else                { lvl = 5; base_delay =  90; }

    PORTD = seg[lvl];

    if (lvl != current_level) {
        current_level = lvl;
        printf("Current Speed: %d\r\n", current_level);
    }
}

// ---------------------------------------------------------------------------
// Utility
// ---------------------------------------------------------------------------
void Print_Score(void) {
    printf("Score - P1: %d | P2: %d\r\n", score_p1, score_p2);
}

void Reset_Game(unsigned char first_server) {
    score_p1      = 0;
    score_p2      = 0;
    game_state    = first_server;
    ball_position = (first_server == 1) ? 6 : 0;  // P1 at pos 6, P2 at pos 0
    printf("\r\n=====================================\r\n");
    printf("       NEW PING PONG MATCH BEGINS      \r\n");
    printf("=====================================\r\n");
    Print_Score();
}

void Check_Win_Condition(void) {
    if (score_p1 >= 10) {
        printf("\r\n*** MATCH OVER! PLAYER 1 WINS! ***\r\n");
        Reset_Game(1);   // P1 scored winning point → P1 serves
    } else if (score_p2 >= 10) {
        printf("\r\n*** MATCH OVER! PLAYER 2 WINS! ***\r\n");
        Reset_Game(2);   // P2 scored winning point → P2 serves
    }
}

// ---------------------------------------------------------------------------
// Button helpers
// ---------------------------------------------------------------------------
unsigned char Read_P1(void) {
    if (PORTCbits.RC0 == 0) {
        __delay_ms(15);
        if (PORTCbits.RC0 == 0) {
            while (PORTCbits.RC0 == 0);
            __delay_ms(15);
            return 1;
        }
    }
    return 0;
}

unsigned char Read_P2(void) {
    if (PORTCbits.RC1 == 0) {
        __delay_ms(15);
        if (PORTCbits.RC1 == 0) {
            while (PORTCbits.RC1 == 0);
            __delay_ms(15);
            return 1;
        }
    }
    return 0;
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------
void main(void) {
    TRISB = 0x00;
    TRISD = 0x00;

    TRISCbits.TRISC0 = 1;
    TRISCbits.TRISC1 = 1;
    TRISAbits.TRISA0 = 1;

    ADCON1 = 0x8E;
    ADCON0 = 0x41;

    UART_Init();

    PORTB = 0x00;
    PORTD = seg[1];

    Reset_Game(1);   // P1 serves first

    unsigned int  t;
    unsigned char hit_success;
    unsigned char early_press;

    while (1) {

        Update_Speed();
        PORTB = (unsigned char)(1 << ball_position);

        // =================================================================
        // STATE 1: SERVE — P1 ball at pos 6 (RB6), no timeout
        // Ball will travel LEFT toward P2 at pos 0
        // =================================================================
        if (game_state == 1) {
            printf("Player 1: Press to serve...\r\n");

            while (Read_P1() == 0) {
                Update_Speed();
                PORTB = (unsigned char)(1 << ball_position);
            }

            printf("Player 1 Serves!\r\n");
            Print_Score();
            ball_position = 5;          // Step away from P1 edge
            game_state    = 3;          // Ball moving LEFT toward P2
        }

        // =================================================================
        // STATE 2: SERVE — P2 ball at pos 0 (RB0), no timeout
        // Ball will travel RIGHT toward P1 at pos 6
        // =================================================================
        else if (game_state == 2) {
            printf("Player 2: Press to serve...\r\n");

            while (Read_P2() == 0) {
                Update_Speed();
                PORTB = (unsigned char)(1 << ball_position);
            }

            printf("Player 2 Serves!\r\n");
            Print_Score();
            ball_position = 1;          // Step away from P2 edge
            game_state    = 4;          // Ball moving RIGHT toward P1
        }

        // =================================================================
        // STATE 3: Ball moving LEFT (pos 5→1) toward P2
        // P1 already hit — only P2 pressing now is an early foul
        // =================================================================
        else if (game_state == 3) {
            early_press = 0;

            for (t = 0; t < base_delay; t++) {

                if (Read_P2()) {
                    printf("Player 2 Pressed Too Early!\r\n");
                    score_p1++;
                    Print_Score();
                    Check_Win_Condition();
                    ball_position = 6;
                    game_state    = 1;  // P1 scored → P1 serves
                    early_press   = 1;
                    break;
                }

                __delay_ms(1);
            }

            if (!early_press) {
                ball_position--;        // Move LEFT

                if (ball_position <= 0) {
                    ball_position = 0;
                    game_state    = 5;  // Ball reached P2 edge (pos 0)
                }
            }
        }

        // =================================================================
        // STATE 4: Ball moving RIGHT (pos 1→5) toward P1
        // P2 already hit — only P1 pressing now is an early foul
        // =================================================================
        else if (game_state == 4) {
            early_press = 0;

            for (t = 0; t < base_delay; t++) {

                if (Read_P1()) {
                    printf("Player 1 Pressed Too Early!\r\n");
                    score_p2++;
                    Print_Score();
                    Check_Win_Condition();
                    ball_position = 0;
                    game_state    = 2;  // P2 scored → P2 serves
                    early_press   = 1;
                    break;
                }

                __delay_ms(1);
            }

            if (!early_press) {
                ball_position++;        // Move RIGHT

                if (ball_position >= 6) {
                    ball_position = 6;
                    game_state    = 6;  // Ball reached P1 edge (pos 6)
                }
            }
        }

        // =================================================================
        // STATE 5: Ball at P2 edge (pos 0) — P2 must HIT within timeout
        // =================================================================
        else if (game_state == 5) {
            hit_success = 0;

            for (t = 0; t < base_delay; t++) {
                if (Read_P2()) {
                    hit_success = 1;
                    break;
                }
                __delay_ms(1);
            }

            if (hit_success) {
                printf("Player 2 Hits!\r\n");
                Print_Score();
                ball_position = 1;      // Step away from P2 edge
                game_state    = 4;      // Ball now moving RIGHT toward P1
            } else {
                printf("Player 2 Misses!\r\n");
                score_p1++;
                Print_Score();
                Check_Win_Condition();
                ball_position = 6;
                game_state    = 1;      // P1 scored → P1 serves
            }
        }

        // =================================================================
        // STATE 6: Ball at P1 edge (pos 6) — P1 must HIT within timeout
        // =================================================================
        else if (game_state == 6) {
            hit_success = 0;

            for (t = 0; t < base_delay; t++) {
                if (Read_P1()) {
                    hit_success = 1;
                    break;
                }
                __delay_ms(1);
            }

            if (hit_success) {
                printf("Player 1 Hits!\r\n");
                Print_Score();
                ball_position = 5;      // Step away from P1 edge
                game_state    = 3;      // Ball now moving LEFT toward P2
            } else {
                printf("Player 1 Misses!\r\n");
                score_p2++;
                Print_Score();
                Check_Win_Condition();
                ball_position = 0;
                game_state    = 2;      // P2 scored → P2 serves
            }
        }
    }
}