import random
import pyled
import time

oscdist = 20
brightness = 75

def main():
    global oscdist, brightness

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
                
            if oscdir[led] == 'up':
                current[led]+=1
                if current[led] >= setpoints[led]+oscdist:
                    oscdir[led] = 'down'
            else:
                current[led]-=1
                if current[led] <= setpoints[led]-oscdist:
                    oscdir[led] = 'up'
            controller.set_status(led, current[led], brightness)


if __name__ == "__main__":
    main()


