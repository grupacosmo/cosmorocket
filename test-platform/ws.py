#!/usr/bin/env python

from websockets.sync.client import connect
import csv
import json
import sys

import matplotlib.animation as animation

import matplotlib.pyplot as plt
import matplotlib.dates
import datetime as dt

fig = plt.figure()
ax = fig.add_subplot(1, 1, 1)
xs = []
ys = []
def main():
    ip_addr = input("IP addr: ")
    uri = f"ws://{ip_addr}/ws"
    with connect(uri) as websocket:
        with open('sensor-readings', 'w') as f:
            def animate(i, xs, ys):

                message = websocket.recv()
                xs.append(dt.datetime.now().isoformat(sep=':', timespec="milliseconds"))

                message = json.loads(message)
                pressure = float(message['Pa'])
                f.write(f"{pressure}\n")
                
                ys.append(float(message["Pa"]))
                

                xs = xs[-20:]
                ys = ys[-20:]

                ax.clear()
                ax.plot(xs, ys)

                plt.xticks(rotation=45, ha='right')
                plt.subplots_adjust(bottom=0.30)
                plt.title('Pressure above 1atm')
                plt.ylabel('Pressure [Pa]')

            ani = animation.FuncAnimation(fig, animate, fargs=(xs, ys), interval=0)
            plt.show()


if __name__ == '__main__':
    main()
