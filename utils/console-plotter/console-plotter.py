import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import serial
import re


fig, ax = plt.subplots()
x_line, = ax.plot(np.random.rand(10))
y_line, = ax.plot(np.random.rand(10))
z_line, = ax.plot(np.random.rand(10))

ax.set_ylim(-360, 360)

t_data = [0]*100
dx_data = [0]*100
dy_data = [0]*100
dz_data = [0]*100

raw = serial.Serial("/dev/ttyUSB0", 115200)
plot_line_pattern = re.compile(r"CONSOLE_PLOTTER: \[(.*)\]")

# raw.open()

def run(data):
    t, dx, dy, dz = data
    del t_data[0]
    del dx_data[0]
    del dy_data[0]
    del dz_data[0]
    t_data.append(t)
    dx_data.append(dx)
    dy_data.append(dy)
    dz_data.append(dz)

    ax.set_xlim(min(t_data), max(t_data))
    x_line.set_data(t_data, dx_data)
    y_line.set_data(t_data, dy_data)
    z_line.set_data(t_data, dz_data)

    print("Done")
    
    return x_line, y_line, z_line

def data_gen():
    t = 0
    while True:
        t+=0.1
        raw_line = str(raw.readline())
        match = plot_line_pattern.search(raw_line)
        print(raw_line)

        if match:
            split_vals = match.group(1).split(",")
            dx = float(split_vals[0])
            dy = float(split_vals[1])
            dz = float(split_vals[2])
        else:
            dx = 0
            dy = 0
            dz = 0

        yield t, dx, dy, dz

ani = animation.FuncAnimation(fig, run, data_gen, interval=0, blit=True)
plt.show()