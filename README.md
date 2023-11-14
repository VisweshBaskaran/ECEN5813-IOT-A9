# ecen5823-f22-assignments
Starter code based on Gecko SDK 3.2.3

I thank the instructor and all the TA's for thier assistance
Device name to look for in EFR connect app: VisweshB

## Usage Instructions:

Change the Macro DEVICE_IS_BLE_SERVER to 1 for server code and 0 for client code before flashing onto the EFR32BG13 dev board
Change the macro SERVER_BT_ADDRESS to your server device's bluetooth address.

 PB0 to be pressed and released slowly, at a rate of 1 press per second since soft timer's period is 1 second