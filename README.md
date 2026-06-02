# Reklamtavlan - Programmering för inbyggda system

Embedded LCD Billboard System
An embedded advertising billboard system implemented in pure C for the ATmega328P (Arduino Uno). The system cycles through client advertisements on a 16x2 LCD display, using a weighted random algorithm to ensure higher-paying clients receive proportionally more screen time.

🚀 Features
Weighted Random Selection: Clients paying more (e.g., 5000 kr) appear significantly more often than those paying less (e.g., 1500 kr).
Anti-Repetition Logic: The system guarantees that the same client is never displayed twice in a row.
Dynamic Text Effects: Supports Static, Scrolling, and Blinking text effects based on client requirements.
Special Business Logic: Implements specific rules for "Svarte Petter" (alternating ads based on even/odd minutes).
Non-Blocking Architecture: Uses hardware timers (Timer1) for precise 20-second intervals without blocking the main loop.
Pure C Implementation: No Arduino libraries (<Arduino.h>); uses direct register manipulation (<avr/io.h>) for optimal performance and educational value.
🛠️ Hardware Specifications
Component	Specification	Notes
Microcontroller	ATmega328P (Arduino Uno)	16 MHz Clock, 32KB Flash, 2KB SRAM
Display	HD44780 16x2 LCD	Operated in 4-bit mode to save GPIO pins
Backlight	LED with 560Ω Resistor	Limits current to ~5.3mA (dimmer but safe)
Contrast	10kΩ Potentiometer	Connected to LCD Pin 3 (V0) for adjustable contrast
Power	5V USB / Barrel Jack	Stable 5V supply required
⚡ Pin Configuration (PORTD)
The LCD is connected using 4-bit data mode to minimize pin usage.

LCD Pin	Function	ATmega328P Pin	PORT Bit
1	VSS (GND)	GND	-
2	VDD (5V)	5V	-
3	V0 (Contrast)	Potentiometer (Wiper)	-
4	RS (Register Select)	Pin 0 (RX)	PD0
5	RW (Read/Write)	GND (Hardwired)	-
6	E (Enable)	Pin 1 (TX)	PD1
11	D4	Pin 4	PD4
12	D5	Pin 5	PD5
13	D6	Pin 6	PD6
14	D7	Pin 7	PD7
15	A (Backlight+)	5V via 560Ω Resistor	-
16	K (Backlight-)	GND	-
Note: Pins 7, 8, 9, and 10 (D0-D3) are not connected as we operate in 4-bit mode.

💻 Software Architecture
1. Timer-Driven Main Loop
Instead of using delay(), the system uses Timer1 in CTC mode to generate a 1ms system tick (system_ticks).

Benefit: The main loop remains responsive, and timing is precise.
Logic: The system checks if (current_time - start_time >= 20000ms) to trigger the next ad.
2. Weighted Random Algorithm
Clients are stored in a struct array with a payment field.

Total Weight: Sum of all payments (e.g., 14,500 kr).
Selection: A random number R (0 to Total Weight) is generated.
Accumulation: The system iterates through clients, accumulating their payment. When Accumulated >= R, that client is selected.
Constraint: The previously selected client is skipped to prevent consecutive repeats.
3. Text Effects Engine
Static: Displays text directly.
Scroll: Shifts the text buffer character by character every 300ms.
Blink: Toggles the display on/off every 1.5s.
4. Special Rules
Svarte Petter: Checks (system_ticks / 60000) % 2. If even, shows "Scroll" ad; if odd, shows "Static" ad.
📦 Building and Uploading
Prerequisites
Compiler: avr-gcc (part of avr-libc)
Uploader: avrdude
Toolchain: Arduino IDE (optional, for serial monitoring) or PlatformIO
Compilation (Pure C)


# Upload via Arduino Bootloader (COM3 on Windows, /dev/ttyUSB0 on Linux)
avrdude -c arduino -p atmega328p -P /dev/ttyUSB0 -b 115200 -U flash:w:billboard.hex
🧪 Testing & Verification
Power Up: The LCD backlight should turn on (dimly due to 560Ω resistor).
Contrast Adjustment: Turn the potentiometer knob until the text becomes sharp and readable.
Observation:
Wait 20 seconds. The ad should change automatically.
Verify that "Harry" (5000 kr) appears more frequently than "Petter" (1500 kr).
Verify that the same ad never appears twice in a row.
Serial Debug (Optional): If using Wokwi or a serial monitor, the system prints debug logs showing the weighted calculation and selected client.
🐛 Troubleshooting
Symptom	Possible Cause	Solution
Screen is blank	Incorrect contrast	Adjust the potentiometer knob.
Screen is all black blocks	Contrast too high	Turn potentiometer the other way.
Garbage characters	Wrong wiring on D4-D7	Check connections to Pins 4, 5, 6, 7.
Upload fails	RX/TX interference	Unplug wires from Pin 0 & 1, upload, then reconnect.
No backlight	Resistor missing/wrong	Ensure 560Ω resistor is between 5V and Pin 15.
Text doesn't change	Timer not initialized	Check timer_init() and sei() (Global Interrupts).
📝 Design Decisions & Trade-offs
4-bit vs 8-bit Mode: Chose 4-bit to save 4 GPIO pins (D0-D3) for potential future sensors or buttons. Sacrifices slight speed for pin efficiency.
560Ω Resistor: Used a higher resistance than the typical 220Ω to reduce power consumption and heat, resulting in a dimmer but safer backlight suitable for battery-powered scenarios.
No delay(): Implemented a non-blocking state machine using hardware timers. This ensures the system remains responsive and timing is accurate even if other tasks are added.
Pure C: Avoided Arduino libraries to demonstrate understanding of low-level register manipulation (DDRD, PORTD, TCCR1B), which is critical for embedded systems.
📄 License
This project is part of the Programmering för inbyggda system course (IOT25) at STI.