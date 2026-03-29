/*第3题	图形运动程序--源代码及关键源代码注解如下：*/
//模拟小球碰撞
#include<graphics.h>
#include<stdlib.h>
main()
{
    int i,gdriver,gmode, size;
    void *buf;
    gdriver=DETECT;
    initgraph(&gdriver,&gmode,"");
    setbkcolor(BLUE);
    cleardevice(); /*清除屏幕*/
    setcolor(LIGHTRED);
    setlinestyle(0,0,1);
    setfillstyle(1,10);
    circle(100,200,30);
    floodfill(100,200,12);
    size=imagesize(69,169,131,231);/*返回存储屏幕所需的字节数*/
    buf=malloc(size); /*动态分配内存空间*/
    getimage(69,169,131,231,buf); /*保存指定位置的小球图*/
    putimage(500,169,buf,COPY_PUT);/*在另一位置上复制小球*/
    while(!kbhit())
    {
    for(i=0;i<185;i++)
    {  delay(400);/*延迟时间，使运动变慢*/
       putimage(70+i,170,buf,COPY_PUT); /*左边球右移 */
       putimage(500-i,170,buf,COPY_PUT);/*右边球左移*/
    }
    for(i=0;i<185;i++)
    {
       delay(400);
       putimage(255-i,170,buf,COPY_PUT);
       putimage(315+i,170,buf,COPY_PUT);
    }
    }
    getch();
    closegraph();
}
