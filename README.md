# Ping-Pong-Game-Project
🏓 2-Player LED Ping Pong Game (PIC16F877A) — An embedded C game using a PIC16F877A, 7 LEDs, two push-buttons, and a potentiometer. An FSM controls ball movement and scoring, ADC input adjusts game speed, and UART logs events in real time. The first player to reach 10 points wins, after which the game resets automatically.
### **Project Description**

This application recreates a two-player ping pong game on a PIC16F877A microcontroller. It demonstrates real-time player interaction, score tracking, adjustable game speed, and serial communication using embedded systems concepts such as finite state machines (FSMs), ADC interfacing, and UART communication.

### **Tech Stack**

* Embedded C
* MPLAB IDE / XC8 Compiler
* PIC16F877A Microcontroller
* UART Serial Communication
* ADC (Analog-to-Digital Converter)

### **Core Features**

* LED-based ball movement across 7 LEDs
* Two-player button-controlled gameplay
* Finite State Machine (FSM) game logic
* Adjustable speed using a potentiometer
* Seven-segment display for speed indication
* Score tracking up to 10 points
* Automatic game reset after a win
* Real-time UART event logging

### **Setup Instructions**

1. Open the project in MPLAB IDE/XC8.
2. Compile and flash the code to a PIC16F877A microcontroller.
3. Connect 7 LEDs to PORTB, buttons to RC0 and RC1, and a potentiometer to AN0.
4. Connect a UART-to-USB adapter for serial monitoring.
5. Power the circuit and start playing by serving the ball with either player button.
