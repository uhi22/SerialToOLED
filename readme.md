
# Serial To OLED

 OLED Display with Serial input or CAN input


# Hardware
- HTIT-WB32, also known as heltec wifi kit 32.
- Arduino board name: WiFi Kit 32
- Version for Arduino IDE 2.0.4 (March 2023) together with Heltec board "Heltec ESP32 Series Dev-boards by Heltec" Version 0.0.7 From board manager url https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series/releases/download/0.0.7/package_heltec_esp32_index.json

# Functionality
- Listens on the serial port (pin18) and on reception of 0x0A it shows the line on the OLED display. 
- Scrolls the older lines up.
- Alternatively, listens to CAN bus, decodes CAN messages and shows signals
