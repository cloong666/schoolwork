/*第3题	图形运动程序--源代码及关键源代码注解如下：*/
//小车运动
#include"stdio.h"
#include"graphics.h"
#include"dos.h"
void main()
    {
    int gd=DETECT,i=0,j,k=0;
    initgraph(&gd,&gd,"\\tc\\bgi");
    while(!kbhit())/*不按键返回0 */
        {
	for( j=0;j<2;j++)
            	{
		 for(i=0;i<500;i+=5)
                	{
                	setcolor(RED);
                	line(118+i,65+j*300,160+i,65+j*300);
                	line(120+i,70+j*300,160+i,70+j*300);
                	line(60+i,100+j*300,220+i,100+j*300);
                	line(200+i,100+j*300,160+i,70+j*300);
                	line(205+i,100+j*300,160+i,65+j*300);
                	line(80+i,100+j*300,120+i,70+j*300);
                	line(75+i,100+j*300,118+i,65+j*300);
                	setfillstyle(SOLID_FILL,RED);
                	floodfill(77+i,99+j*300,RED);
                	line(135+i,70+j*300,135+i,100+j*300);
                	line(145+i,70+j*300,145+i,100+j*300);
                	setfillstyle(SOLID_FILL,RED);
                	floodfill(137+i,75+j*300,RED);

                	setfillstyle(SOLID_FILL,WHITE);
                	floodfill(130+i,75+j*300,RED);
                	setfillstyle(SOLID_FILL,WHITE);
                	floodfill(150+i,75+j*300,RED);
             /*rectangle(left,top,right,bottom); */
               /*	rectangle(80+i,90+j*300,100+i,110+j*300); */
                	line(150+i,90+j*300,160+i,80+j*300);
                	line(154+i,90+j*300,164+i,80+j*300);
                	line(158+i,90+j*300,168+i,80+j*300);
                	line(110+i,90+j*300,120+i,80+j*300);
                	line(114+i,90+j*300,124+i,80+j*300);
                	line(118+i,90+j*300,128+i,80+j*300);
                	line(20+i,140+j*300,260+i,140+j*300);
			/* arc(midx, midy, stangle, endangle, radius); */
                	arc(200+i,140+j*300,0,177,25);
                	arc(80+i,140+j*300,0,177,25);
                	arc(60+i,140+j*300,90,178,40);
                	arc(220+i,140+j*300,0,90,40);
                	setfillstyle(SOLID_FILL,RED);
                	floodfill(100+i,110+j*300,RED);
                	/*边框黑色*/
                	setcolor(0);
                	line(139+i,70+j*300,139+i,220+j*300);
                	line(140+i,70+j*300,140+i,220+j*300);
                	line(141+i,70+j*300,141+i,220+j*300);
                	setcolor(RED);
                	/*车灯*/
                	setcolor(YELLOW);
                	circle(250+i,130+j*300,5);
                	line(253+i,125+j*300,312+i,113+j*300);
                	line(253+i,135+j*300,310+i,143+j*300);
                	arc(315+i,128+j*300,250,100,15);
                	/*EMPTY_FILL,SOLID_FILL,LINE_FILL,LTSLASH_FILL,SLASH_FILL,
                	BKSLASH_FILL,LTBKSLASH_FILL,HATCH_FILL,XHATCH_FILL,
                	INTERLEAVE_FILL,WIDE_DOT_FILL,CLOSE_DOT_FILL,*/
                	setfillstyle(LTBKSLASH_FILL,YELLOW);
                	floodfill(280+i,130+j*300,YELLOW);
                	/*排气管 */
                	rectangle(15+i,137+j*300,20+i,140+j*300);
                	setcolor(BROWN);
                	circle(15+i-k,135+j*300,k);
                	setfillstyle(INTERLEAVE_FILL,BROWN);
                	floodfill(15+i-k,135+j*300,BROWN);
                	if(k>20) 	 k=2;
                	else	 k+=5;
                	setfillstyle(SOLID_FILL,15);
                	floodfill(190+i,120+j*300,RED);
                	floodfill(80+i,120+j*300,RED);
                	/*轮胎*/
                	setcolor(DARKGRAY);
                	circle(200+i,140+j*300,18);
                	circle(80+i,140+j*300,18);
                	setfillstyle(SOLID_FILL,DARKGRAY);
                	floodfill(190+i,140+j*300,DARKGRAY);
                	floodfill(80+i,140+j*300,DARKGRAY);
                	setcolor(RED);
                	circle(200+i,140+j*300,10);
                	circle(80+i,140+j*300,10);
                	setfillstyle(SOLID_FILL,RED);
                	floodfill(205+i,140+j*300,RED);
                	floodfill(85+i,140+j*300,RED);
                	clearviewport();/*清除窗口信息*/
                	if(kbhit()) break;
                	}
                	}
            }
            closegraph();
        }
