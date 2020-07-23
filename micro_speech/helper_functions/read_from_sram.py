#This script might be useful to test the I2S driver.

a = 0x200108E4 #Change this address based on where in the ram the sound is saved. In zephyr, something like: printk("%u",&buffer_name);

#Remember to download these python libraries
from pynrfjprog import API 
import pyperclip
api = API.API('NRF53')
api.open()
api.connect_to_emu_without_snr(jlink_speed_khz=1000)
buffer1 = api.read(a,1+4*512)
api.close()

j=0
tmp=""
tmp_byte=""
output=""
x=0
summ=0

#Since the pynrfjprog read function returns max int8_t at a time, we have to turn them around and patch them together and such to get the actual int32_t we saved there.
for i in buffer1:
    if(j==4):
        j=0
        x=0
        x = int("0x"+ (tmp[::-1]),16)
        if (x >= 2147483648):
            x -= 4294967296
        output+= str(x)+ ", "
        tmp=""
    tmp_byte=str("{0:#0{1}x}".format(i,4)[2:][::-1]) #arcane python
    tmp+=tmp_byte
    j+=1

#Paste the formatted string with numbers to the clipboard.
#We used https://sooeet.com/math/online-fft-calculator.php to look at waves and frequensies
pyperclip.copy(output)
