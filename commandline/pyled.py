import os
import random
import ctypes

class usb_dev_handle(ctypes.Structure):
    pass

# Pointers
usb_dev_handle_p = ctypes.POINTER(usb_dev_handle)

class LEDController(object):
    def __init__(self):
        self.p = usb_dev_handle_p()
        self.ledlib = ctypes.cdll.LoadLibrary(os.getcwd() + '/led.so')
        self.p = self.ledlib.my_init(self.p)

    def print_status(self):
        self.ledlib.get_pwm_status(self.p)

    def set_status(self, led, hue, brightness):
        assert 0 <= led <= 2
        #assert 0 <= hue <= 252
        #assert 0 <= brightness <= 255
        self.ledlib.set_pwm_status(self.p, str(led), str(hue), str(brightness))

