# bmjpngViewer
A picture viewer for psvita 
----------------------------------------------------------------------------------------
to build this you have to install vdpm first.
1. install vdpm.  reference:  https://github.com/vitasdk/vdpm
2. after installed vdpm, to build  "ComicViewer.vpk" just make.
3. copy ComicViewer.vpk to PSVita and install ComicViewer.vpk.
Enjoy!
----------------------------------------------------------------------------------------

一个简单的看图程序。
1. 在ux0:data/comic目录下，建立目录，然后把图片放进里面，就可以进入bmjpngViewer对图片进行查看了。
   在vitaShell查看目录应该如下 ux0:data/comic/图片目录1， ux0:data/comic/图片目录2 等。
2. 支持 BMP JPG PNG 三种后缀的图片。
3. 图片尺寸最好小于2000*2000，太大的图片占内存大且打开等待时间长，体验不太好。

用法：
1. 从这里的release中，下载vpk程序，复制到psvita，然后在vitaShell中选中文件，确定并安装。
2. 进入程序后，SELECT键时打开或关闭帮助信息。在目录选择界面，按方框键，也有相关的信息。
3. 需要背景图片的话，请存成bg.png格式，文件如下：ux0:data/comic/bg.png
4. 看图时，三角是高度适合图片，方框是宽度适合，圆圈是按实际大小显示。左摇杆移动图片，右摇杆上下是放大缩小。点击触屏可跳到想要的图片。
5. 叉是返回，圆圈是确定，按START退出程序。

程序BUG：
1. 一些图片虽然是支持的格式，但打开后，程序会崩溃，严重的GPU驱动崩溃。需要长按电源键20秒，重新启动机器。
2. 图片尺寸超2000*2000的，或大小在1.3M以上。容易使程序出BUG，主要原因是占用太多内存，导致无法再加载下一张图片。
   当图片尺寸过大时，程序上方会用红字显示出当前图片的尺寸。
