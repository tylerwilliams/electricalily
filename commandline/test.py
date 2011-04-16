import pyled

if __name__ == "__main__":
    controller = pyled.LEDController()
    controller.print_status()

    for i in xrange(250):
        controller.set_status(0, i, i)
        controller.set_status(1, i, i)
        controller.set_status(2, i, i)

    controller.set_status(0, 0, i)
    controller.set_status(1, 0, i)
    controller.set_status(2, 0, i)
