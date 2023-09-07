Please include your answers to the questions below with your submission, entering into the space below each question
See [Mastering Markdown](https://guides.github.com/features/mastering-markdown/) for github markdown formatting if desired.

**1. How much current does the system draw (instantaneous measurement) when a single LED is on with the GPIO pin set to StrongAlternateStrong?**
   Answer: 5.34 mA. (Ranges between 4.74 mA and 5.34 mA)


**2. How much current does the system draw (instantaneous measurement) when a single LED is on with the GPIO pin set to WeakAlternateWeak?**
   Answer: 5.27 mA (Ranges between 4.70 mA to 5.27 mA)


**3. Is there a meaningful difference in current between the answers for question 1 and 2? Please explain your answer, referencing the main board schematic, WSTK-Main-BRD4001A-A01-schematic.pdf, and AEM Accuracy in the ug279-brd4104a-user-guide.pdf. Both of these PDF files are available in the ECEN 5823 Student Public Folder in Google drive at: https://drive.google.com/drive/folders/1ACI8sUKakgpOLzwsGZkns3CQtc7r35bB?usp=sharing . Extra credit is available for this question and depends on your answer.**
   Answer: There is not a meaningful difference in current between the answers for questions 1 and 2. The difference is < 0.1 mA when the LED is turned on in both the drive strength. 
   
   With two different drive strength modes, we still see comparable values to understand this we need to look into drive strength. Drive strength corresponds to the maximum current an output pin can source to the load. At the hardware level, there are multiple transistors (with different on resistances) in parallel that drives a pin high or low. With higher drive strength, more of these transistors turn on, which corresponds to lesser internal resistance and will then be able to source more current to the load. 

   I percieve the drive strength modes as "can drive currents upto" modes. As mentioned in the gpio.c comments, the StrongAlternateStrong can drive upto 10mA and the WeakAlternateWeak mode can drive upto 1mA. So, if a load requires a specific amount of current to function, it will still draw the same amount of current. But if that was the the case, the WeakAlternateWeak mode should not be able to drive more than 1mA and we see over 5mA (question 2). As AEM monitors VMCU, it measures the current the board altogether is drawing. Hence I ran the profiler without turning on the LED and noticed a close to constant current of 4.98 mA (average). It means the LED draws only around 0.3mA. This is why both the drive strength modes are able to drive the LED. But since the load remains the same, there should not be even the 0.1 mA difference, this small difference between the current drawn in the two drive strength modes is due to AEM accuracy as for BRD4001A, when measuring currents above 250 uA, the accuracy is within 0.1 mA and that is the difference we see. 

   Also apart from the magnitude I noticed a slew rate change with the rise and fall times, with the WeakAlternateWeak drive strength mode, the rise time and fall time were 600us and 500 us respectively and in case of StrongAlternateStrong 500us and 300us respectively. 



**4. With the WeakAlternateWeak drive strength setting, what is the average current for 1 complete on-off cycle for 1 LED with an on-off duty cycle of 50% (approximately 1 sec on, 1 sec off)?**
   Answer: 4.96 mA


**5. With the WeakAlternateWeak drive strength setting, what is the average current for 1 complete on-off cycle for 2 LEDs (both on at the time same and both off at the same time) with an on-off duty cycle of 50% (approximately 1 sec on, 1 sec off)?**
   Answer: 5.37 mA


