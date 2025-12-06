# Debugging UART with Rigol DS1054 Oscilloscope

This guide explains how to use your Rigol DS1054Z oscilloscope to decode the UART communication between the ESP32 and the Nextion display.

## 1. Hardware Connections

Connect your oscilloscope probes to the ESP32 pins:

*   **Channel 1 (TX)**: Connect to **GPIO17** (ESP32 TX -> Nextion RX). This shows commands sent *from* ESP32 *to* Nextion.
*   **Channel 2 (RX)**: Connect to **GPIO16** (ESP32 RX <- Nextion TX). This shows responses/events *from* Nextion *to* ESP32.
*   **Ground**: Connect the probe ground clips to a **GND** pin on the ESP32.

## 2. Oscilloscope Configuration

### Vertical (Voltage)
*   **Scale**: Set both channels to **1.00 V/div** or **2.00 V/div** (Signal is 3.3V logic).
*   **Position**: Offset the channels so they don't overlap (e.g., CH1 in upper half, CH2 in lower half).

### Horizontal (Time)
*   **Scale**: Set to **1.00 ms/div** or **500 us/div**.
    *   *Calculation*: At 9600 baud, one bit is ~104 µs. A byte (10 bits) is ~1 ms.

### Trigger
*   **Source**: **CH1** (to see commands sent to display).
*   **Mode**: **Normal** (not Auto, so it captures intermittent packets).
*   **Type**: **Edge** (Falling edge usually works best for UART start bits).
*   **Level**: **~1.6 V** (Middle of 3.3V logic).

## 3. Decode Setup (RS232/UART)

1.  Press the **MATH** button (or **Decode** if dedicated).
2.  Select **Decode1** (or Decode2).
3.  **Decode**: Turn **ON**.
4.  **Type**: Select **RS232** (UART).
5.  **Source**:
    *   **TX**: Set to **CH1**.
    *   **RX**: Set to **CH2** (if decoding both).
6.  **Settings**:
    *   **Baud**: **9600**
    *   **Data Bits**: **8**
    *   **Parity**: **None**
    *   **Stop Bits**: **1**
    *   **Polarity**: **Normal** (Idle High) - *Important!* UART idles at 3.3V (High) and drops to 0V (Low) for the start bit.
7.  **Format**: **ASCII** (to read the text commands like `temp.val=22`).

## 4. What to Look For

### Channel 1 (ESP32 -> Nextion)
You should see ASCII text commands appearing every few seconds (based on update interval).

**Expected Packet:**
`zero.temp.val=22` followed by three `0xFF` bytes (displayed as `ÿÿÿ` or hex `FF FF FF`).

**Troubleshooting:**
*   **Flat Line (3.3V)**: No data is being sent. Check ESPHome logs to see if it *thinks* it's sending.
*   **Garbage Characters**:
    *   Check **Baud Rate** (must match 9600).
    *   Check **Polarity** (try Inverted if Normal fails, but ESP32 is usually Normal).
    *   Check **Ground** connection.
*   **Voltage Levels**: Ensure peaks are ~3.3V. If 0V or 5V, check wiring.

### Channel 2 (Nextion -> ESP32)
You should see data when you touch the screen or when the display initializes.

**Expected Packet:**
Events like `e` (0x65) followed by page/component IDs.

## 5. Capturing a Specific Event

To capture the initialization sequence:
1.  Set Trigger to **Single** mode.
2.  Press the **Reset** button on the ESP32.
3.  The scope should trigger on the first burst of data (the `cls` or `page 0` commands).
