# import sin cos
from math import sin, cos
import time

file = "/tmp/test_data"

with open(file, "w") as f:
    t = 0
    while True:
        v1 = sin(t / 10.0) * 100
        v2 = cos(t / 10.0) * 20
        # lc,t,v1
        # ps,t,v2
        f.write(f"lc,{t},{v1}\n")
        f.write(f"ps,{t},{v2}\n")
        f.flush()
        t += 1
        time.sleep(0.01)

