# BLuetooth low-cost oscilloscope hardware

This firmware was developed for a graduation project for the Computer Enginnering 
course at University of São Paulo (USP), São Carlos campus. The title of the 
project is *Implementation of a low cost Oscilloscope with graphical display on 
an Android application*, and the final report (in Brazilian Portuguese) is available 
[here](http://www.tcc.sc.usp.br/tce/disponiveis/97/970010/tce-04012017-163919/).

This firmware is used on the hardware unity of the project. An ARM Cortex M4F 
microcontroller (Texas Instruments LM4F120H5QR) was used for constinuosly reading
and pre-processing an analog signal and send it via Bluetooth (SPP profile) to
a smartphone application ([repository](https://github.com/jeajjr/tcc-android)).

The hardware set-up is plain, consisting of a Texas Instruments Stellaris launchPad
development kit for the Cortex M4F and the Arduino-style Bluetooth module HC-05.
A special application layer protocol was developed so the microcontroller would
work in two communication states, one for a constinuous read-and-send of the 
signal and another for time snapshots when using voltage level triggering.