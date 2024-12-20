## 相机成像的几个因素
- Sensor size: 传感器尺寸，决定了成像的分辨率
- Focal length: 焦距，在固定的Sensor size下，决定了fov的大小
- Aperture: 光圈，决定了单位时间内进光的多少，也就是光线强度
- Shutter speed: 快门速度，指的是快门打开的时间，决定了曝光时间
- ISO： 感光度，可以简单理解为最后成像亮度的一个系数乘，会同时放大像素的颜色值和噪点

照片的最后形成是一段时间内光线在传感器上的作用，因此最后成像的亮度由Aperture、Shutter speed、ISO、光源等因素共同决定。

在ISO不变的情况下，你可以通过调节Aperture和Shutter speed来控制亮度的改变，比如小光圈慢快门与大光圈快快门的成像亮度一致

但是Aperture还会影响成像的景深
以及Shutter speed在过慢的情况下会有motion blur的现象

## 光场
光场是全光函数（Plenoptic function）的一小部分
光场定义了某点某方向的光强
因此，看向物体时，其实只需要在摄像机方向上查询物体所对应光场的点的该方向的光强即可知道物体在摄像机上的颜色

光场摄像机： 通过一个微透镜矩阵（每个微透镜背后还有一个数字传感器），记录场景在各个方向上的光线信息，原来的正常照片中一个像素表示场景中的一个像素，而对于光场摄像机来说，可能是一个100*100的像素对应场景中的一个像素，而这个100*100则表示了方向的数量
