# Microcontroller Features - Interrupts Exercise

Steps:
1. Use `PINS_DRV_Init` to initialize all needed pins:
- Pins connected to the RED and GREEN LEDs must be configured as outputs.
- Pins connected to the SW2 and SW3 (buttons) must be configured as inputs.
2. Implement the `buttonISR` function. This function will service the interrupt triggered by the button press. Hint: inside this function, you need to use `PINS_DRV_GetPortIntFlag` to check which button was pressed. In case SW2 was pressed, use `PINS_DRV_TogglePins` to "switch" the state of one of the LEDs. Otherwise, do the same, but for the other LED.
3. Setup the button pin interrupts using `PINS_DRV_SetPinIntSel`.
4. Use `INT_SYS_InstallHandler` to install your own `buttonISR`. This will associate the interrupt vector - a special memory location - with the ISR defined by us.