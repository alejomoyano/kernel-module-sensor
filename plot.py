import matplotlib.pyplot as plt
import matplotlib.animation as animation



def rtgraph(interval):
    global counter
    global x

    pulse = open('/dev/gsensors','r').read()
    # pulse = open('nose.txt','r').read()
    # lines = pulse.split('\n')


    # for p in lines:
    if len(pulse) > 0:
        if pulse == '0': 
            print(counter)
            counter = 0
        else:
            counter += 1
        pulses.append(counter)
    
    x = []
    x = range(1, len(pulses) + 1)
    subplot.clear()
    subplot.plot(x, pulses)

counter = 0
pulses = []
x = 0
figure = plt.figure()
subplot = figure.add_subplot(1,1,1)

ani = animation.FuncAnimation(figure, rtgraph, interval=1000)
plt.show()