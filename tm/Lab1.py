import sympy as s
import matplotlib.pyplot as p
import numpy as n
from matplotlib.animation import FuncAnimation
import math

t = s.Symbol('t')

x = s.sin(t)
y = s.sin(2*t)

Vx = s.diff(x)
Vy = s.diff(y)

step = 1000
T = n.linspace(0, 10, step)

X = n.zeros_like(T)
Y = n.zeros_like(T)
VX = n.zeros_like(T)
VY = n.zeros_like(T)
for i in n.arange(len(T)):
    X[i] = s.Subs(x, t, T[i])
    Y[i] = s.Subs(y, t, T[i])
    VX[i] = s.Subs(Vx, t, T[i])
    VY[i] = s.Subs(Vy, t, T[i])

fgr = p.figure()
graf = fgr.add_subplot(1,1,1)
graf.axis('equal')
graf.set(xlim = [-2,2], ylim = [-2, 2])
graf.plot(X,Y)

Pnt = graf.plot(X[0], Y[0], marker='o')[0]
VP = graf.plot([X[0], X[0]+VX[0]], [Y[0], Y[0]+VY[0]], 'r')[0]

def Vect_arrow(Xa, Ya, X0, Y0):
    a = 0.3
    b = 0.2
    ArX = n.array([-a, 0, -a])
    ArY = n.array([b, 0, -b])

    phi = math.atan2(Ya, Xa)

    RotX = n.cos(phi)*ArX - n.sin(phi)*ArY
    RotY = n.sin(phi)*ArX + n.cos(phi)*ArY

    ArX = RotX + Xa + X0
    ArY = RotY + Ya + Y0
    return ArX, ArY

ArrVX, ArrVY = Vect_arrow(VX[0], VY[0], X[0], Y[0])
ArrV = graf.plot(ArrVX, ArrVY, 'red')[0]

def run(i):
    Pnt.set_data(X[i], Y[i])
    VP.set_data([X[i], X[i]+VX[i]], [Y[i], Y[i]+VY[i]])
    ArrVX, ArrVY = Vect_arrow(VX[i], VY[i], X[i], Y[i])
    ArrV.set_data(ArrVX, ArrVY)
    return

anim = FuncAnimation(fgr, run, frames=step, interval=1, repeat=False)

# Сохраняем анимацию в переменной
# Это предотвращает удаление анимации до завершения отображения
plt.show()
