1. This module creates device tree overlay and addes it at the time of booot to resrve 32 Mb at 4Gb section
2. verify the memory reserve by "sudo dmesg | grep "reserved"
3. add the dtoverlay in cofig.txt file and add the file into /boot/firmware/current/overlays/rtos.dtbo
4. sudo reboot -> verify after reboot if memory is reserved.
