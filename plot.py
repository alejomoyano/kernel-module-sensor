import matplotlib.pyplot as plt
import os
from tkinter import Tk, Button, BOTH, TOP, BOTTOM
from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from time import sleep
from threading import Thread

def change_signal():
    global signal
    global driver

    print('estamos leyendo el sensor',signal)


    figure.clear()
    pulses.clear()
    x.clear()

    if signal == 1:
        signal = 2
        os.write(driver, b'2')
    else:
        signal = 1
        os.write(driver, b'1')

        # driver.flush()
        

def logic():
    global pulses
    while 1:

        pulse = os.read(driver, 1024)
        decoded = pulse.decode('utf-8')

        # print(decoded)
        if len(decoded) > 0:
            pulses.append(decoded)
        print(decoded)

        x = range(1, len(pulses) + 1)

        figure.clear()
        if signal == 1:
            figure.add_subplot(111).step(x,pulses)
        else:
            figure.add_subplot(111).plot(x,pulses)

        canvas.draw()
        sleep(1)


if __name__ == "__main__":

    driver = os.open('/dev/gsensors',os.O_RDWR)

    counter = 0
    pulses = []
    x = []

    signal = 1

    root = Tk()

    global figure
    figure = Figure()

    if signal == 1:
        figure.add_subplot(111).step([],[])
    else:
        figure.add_subplot(111).plot([],[])

    canvas = FigureCanvasTkAgg(figure, master=root) 
    canvas.draw()
    canvas_widget = canvas.get_tk_widget()
    canvas_widget.pack(side=TOP, fill=BOTH, expand=1)

    button = Button(master=root, text="Change", command=change_signal)
    button.pack(side=BOTTOM)

    rep_thread = Thread(target=logic) 
    rep_thread.daemon = True
    rep_thread.start()


    root.mainloop()
