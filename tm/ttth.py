import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import sympy as sp
import matplotlib

matplotlib.use("TkAgg")

t = np.linspace(0, 10, 11)
s = 0.5 * (1 + 0.3 * np.sin(0.8 * t) + 0.5 * np.cos(4.43 * t))

W = 1
w = 0.5
H = 0.5
R = 0.2

xO = W/2 + s
yO = H/2

xT = np.array([0, 0.25, 0.75, 1, 0])
yT = np.array([0, 0.5, 0.5, 0, 0])

Alpha = np.linspace(0, 6.29, 50)

fig = plt.figure(figsize=[13, 9])
ax = fig.add_subplot(1, 1, 1)
ax.axis('equal')
ax.set(xlim=[-0.25, 3], ylim=[-0.25, 2])

ax.plot([0, 0, 2.75], [1.75, 0, 0], color=[0, 0.5, 0], linewidth=4)
Trapec = ax.plot(xO[0]+xT, yT, color=[0, 0, 0])[0]
M = ax.plot([], [], 'o', color=[0, 0.75, 0], markersize=10, markerfacecolor=[0, 1, 0], linewidth=2)[0]
Circle = ax.plot([], [], color='black')[0]

def kadr(i):
    theta = 0.5 * np.pi / (t[-1] - t[0]) * i
    M.set_xdata(xO[i] + 0.5 + R * np.cos(theta))
    M.set_ydata(yO + R * np.sin(theta))
    Circle.set_xdata(xO[i] + 0.5 + R * np.cos(Alpha))
    Circle.set_ydata(yO + R * np.sin(Alpha))
    Trapec.set_data(xO[i] + xT, yT)
    return [M, Circle, Trapec]

kino = FuncAnimation(fig, kadr, interval=t[1] - t[0], frames=len(t))
plt.show()
