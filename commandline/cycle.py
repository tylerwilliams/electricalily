import random
import pyled
import time

brightness = 75

def main():
    global brightness

    setpoints = {0:232, 1:70, 2:150}
    oscdir = {}
    current = {}
    controller = pyled.LEDController()

    while 1:
        for led in range(0,3):
            # if we don't have a set point, pick a random one
            if not led in setpoints:
                setpoints[led] = random.randint(1,252)
            if not led in oscdir:
                oscdir[led] = random.choice(['up', 'down'])
                current[led] = setpoints[led]
                
            current[led]+=1
            if current[led] >= 252:
                current[led] = 1

            controller.set_status(led, current[led], brightness)
        time.sleep(.1)

if __name__ == "__main__":
    main()


