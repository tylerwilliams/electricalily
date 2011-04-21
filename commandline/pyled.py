import os
import random
import ctypes

class LEDController(object):
    def __init__(self):
        self.ledlib = ctypes.cdll.LoadLibrary(os.getcwd() + '/led.so')
        self.ledlib.open_handle()

    def print_status(self):
        self.ledlib.get_pwm_status()

    def set_status(self, led, hue, brightness):
        assert 0 <= led <= 2
        #assert 0 <= hue <= 252
        #assert 0 <= brightness <= 255
        self.ledlib.set_pwm_status(str(led), str(hue), str(brightness))

    def __del__(self):
        self.ledlib.close_handle()
