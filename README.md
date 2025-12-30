Arduino Mega MIDI Loop Switcher (Open Source)

This project is an open-source Arduino Mega 2560–based MIDI loop switcher designed for guitar and bass rigs that need predictable, preset-driven control with optional stompbox-style flexibility.

It combines:

true-bypass relay switching,

per-preset MIDI control,

stompbox and preset modes,

MIDI clock sync via tap tempo,

and a hardware-first UI.

The design goal is simple:
nothing changes unless you press something.

This project is intentionally scoped for the Arduino Mega 2560 due to I/O count, memory headroom, and predictable timing.

Current Features
Core Switching

3–5 true-bypass relay loops (configurable in code)

Relay control via ULN2803A

Optional relay output for amp channel switching

MIDI

MIDI Program Change (PC)

MIDI Control Change (CC)

Per-preset MIDI channel assignment

MIDI clock sync OUT

Tap tempo generates MIDI clock

MIDI OUT only (no MIDI IN)

Presets & Banks

Preset-based architecture (relay states + MIDI data)

Unlimited banks (EEPROM-limited)

Each bank holds multiple presets

Preset changes occur only on footswitch press

Bank navigation does not trigger MIDI or relay changes

Stompbox Mode

Stompbox mode can be enabled per preset

Individual loops can be toggled on/off like traditional pedals

Preset defines:

which loops are active

whether the preset behaves as a “fixed scene” or a stompbox-style layout

No global mode switching required

User Interface

SSD1306 OLED display

Rotary encoder with push button

Dedicated navigation buttons

On-device menu system for:

preset editing

MIDI settings

stompbox configuration

expression calibration

Footswitching

Multiple footswitches for preset selection and loop control

Bank up / bank down switches

Debounced, state-safe logic

No background state mutation

Expression / Control

Expression pedal input (ADC)

Assignable CC output

Heel/toe calibration stored in config

Optional global default EXP CC

Bill of Materials (Core)

Controller & Logic

Arduino Mega 2560

ULN2803A Darlington array

Logic ICs as required (74HC series)

Display & UI

SSD1306 128×64 OLED (I2C)

Rotary encoder with push button

Tactile push buttons

Momentary footswitches (soft-touch)

Switching

Signal relays (e.g. Omron G6K-2 or equivalent)

Power

9V DC input

Buck converter (9V → 5V)

Power filtering:

220µF electrolytic capacitor

0.1µF ceramic decoupling capacitors

I/O

¼″ mono jacks (loops, amp switching)

MIDI OUT (5-pin DIN or TRS MIDI)

Expression pedal jack (TRS)

Indicators

LEDs + current-limiting resistors

Misc

Enclosure (Hammond / equivalent)

Stripboard or custom PCB

Wire, headers, standoffs, hardware

LEGACY CODE FOR OLD ARDUINO BASED MIDI SWITCHER WILL BE LABELED AS LEGACY.
