## 质点弹簧系
感觉是一个非常简化的物理系统

起初对于摩擦力的模拟只是让物体在运动时受到一个与速度相反的力，而且竟然和速度成正比。

但是这样会有问题，就是我们希望模拟的是两个质点间的弹簧状态，对于整个状态系的改变，比如两个点一起往一个方向运动（相对速度为0），我们不希望两个点会因为前面的摩擦力模拟而停下来，我们想要的是在这个弹簧系统内，两个点的运动状态受到弹簧力的影响。

因此，我们需要考虑的是相对速度，而非绝对速度。

## 有限元建模

pass

先模拟后渲染


## 正向运动学
关节有三种，pin（围绕一个点旋转）,ball（可全向旋转）,prismatic joint（可伸缩）

正向运动学需要规定各个关节的旋转角度，拉伸长度，从而得到与关节相连的物块端点的位置

实现挺简单的，但对于艺术家们，一一规定角度，操作不方便

## 反向运动学
反向运动学，则是规定关节上端点的位置，从而反推得到各个关节的旋转角度，拉伸长度

求解很难，而且经常有多解和无解的情况

对于多解的问题，可以采用一些优化方法，比如从当前位置转到下一个目标位置，转动多少角度为最优解？ （牛顿法，梯度下降）

## Rigging
给出控制点，然后艺术家可以操作这些控制点改变人物的形态
## Blend Shape
控制点移动时的角色形态过渡，插值
