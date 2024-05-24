#include "stdlib.h"
#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "gameover.h"
#include "xil_io.h"
#include "xgpio_l.h"
#include "xtmrctr_l.h"
#include "xintc_l.h"
#include "xtft.h"
#include "xparameters.h"

//#define SCREEN_WIDTH  30   //游戏屏幕宽度
//#define SCREEN_LENGTH 20   //游戏屏幕长度
#define START_X 0         //屏幕起始x坐标
#define START_Y 0          //屏幕起始y坐标
#define ADD_NUM 16          //像素块大小

//#define TIMER_BASEADDR 0x41C00000
//基地址的宏定义
#define TFT_BASEADDR XPAR_AXI_TFT_0_BASEADDR
#define TFT_FRAME_BASEADDR XPAR_MIG7SERIES_0_BASEADDR//把定义过的宏应该写在后面
#define RESET_VALUE 100000000-2

#define FOOD_COLOR 0x0000ff00      //绿色
#define BOOM_COLOR 0x00ff0000      //绿色
#define HEAD_COLOR 0x00ff0000     //红色
#define BODY_COLOR 0x00ff00ff		//玫红色
#define WALL_COLOR 0x000000ff     //蓝色
#define SPACE_COLOR 0xffffffff     //白色
#define NAME_COLOR 0xffffffff     //玫红色
#define TFT_DEVICE_ID XPAR_TFT_0_DEVICE_ID
#define TFT_FRAME_BASEADDR0 0X86000000
#define TFT_FRAME_BASEADDR1 0X84000000
#define Btn_UP 0x2
#define Btn_DOWN 0x10
#define Btn_LEFT 0x4
#define Btn_RIGHT 0x8
#define Btn_CENTER 0x1

int shapeIndex=3, shapeIndex_next;//方块索引，取值为1-19

static XTft TftInstance;     //实例化tft
XTft_Config* TftConfigPtr;
int ledth = 0;
const int HEIGHT_1 = 20;
const int HEIGHT_2 = 30;
const int WIDTH = 20;
int TIME_PAUSE = 50000;
//int SPEED_UP = 800000;
int score=0;//得分
int check=1;//用于判断游戏是否结束
char speed_down=1;//速度因子
char speed_max=3;//速度等级

int a=2;//随机种子

enum thing
{
	WALL,
	SPACE,
    move_square,//1
	fixed_square,//2
	NAME
};
struct Point {//中心点的偏移量，中心点以此延伸获得各形状点的坐标
	int shape_x;
	int shape_y;
};
struct Point shapes[19][4] = {//四组，每一组对应输出一个方块
    {{0, 0},{-1, 0},{1, 0},{2, 0}},      //横条
    {{0, 0},{0, -1},{0, 1},{0, 2}},      //竖条
    {{0,0},{-1,-1},{-1,0},{0,-1}},     //方块
    {{0, 0},{0, -1},{0, -2},{1, 0}},    //正Ｌ１
    {{0,0},{0,1},{1,0},{2,0}},           //正Ｌ２
    {{0,0},{-1,0},{0,1},{0,2}},         //正Ｌ３
    {{0,0},{0,-1},{-1,0},{-2,0}},      //正Ｌ４
    {{0,0},{-1,0},{0,-1},{0,-2}},      //反Ｌ１
    {{0,0},{0,-1},{1,0},{2,0}},         //反Ｌ２
    {{0,0},{1,0},{0,1},{0,2}},           //反Ｌ３
    {{0,0},{-1,0},{-2,0},{0,1}},        //反Ｌ４
    {{0,0},{-1,0},{1,0},{0,-1}},        //Ｔ１
    {{0,0},{0,1},{0,-1},{1,0}},         //T 2
    {{0,0},{-1,0},{1,0},{0,1}},         //T 3
    {{0,0},{-1,0},{0,-1},{0,1}},        //T 4
    {{0,0},{1,0},{0,-1},{-1,-1}},      //正 Z 1
    {{0,0},{1,-1},{0,1},{1,0}},         //正Ｚ２
    {{0,0},{1,-1},{-1,0},{0,-1}},      //反ｚ１
    {{0,0},{-1,-1},{-1,0},{0,1}}       //反Ｚ２
};
int begin=0;
int map[30][20];
//int lastmap[30][20];
int pos=0;
char segtable[16]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x98,0x88,0x83,0xc6,0xa1,0x86,0x8e};
char poscode[8]={0x7f,0xbf,0xdf,0xef,0xf7,0xfb,0xfd,0xfe};
char segcode[8]={0xff,0xff,0xff,0xff,0xff,0xff,0xc0,0xc0};
char score0[12][2] = {{23,11},{23,12},{23,13},{23,14},{23,15},{24,11},{24,15},{25,11},{25,12},{25,13},{25,14},{25,15}};
char score1[5][2] = {{25,11},{25,12},{25,13},{25,14},{25,15}};
char score2[11][2] = {{9,12},{9,13},{9,14},{11,14},{10,14},{11,13},{11,12},{12,12},{13,12},{13,13},{13,14}};
char score3[11][2] = {{9,12},{9,13},{9,14},{11,14},{10,14},{11,13},{11,12},{12,14},{13,14},{13,13},{13,12}};
char score4[9][2] = {{9,12},{10,12},{11,12},{11,13},{11,14},{9,14},{10,14},{12,14},{13,14}};
char score5[11][2] = {{9,12},{9,13},{9,14},{11,14},{10,12},{11,13},{11,12},{12,14},{13,14},{13,13},{13,12}};
char score6[10][2] = {{9,12},{10,12},{11,12},{12,12},{13,12},{11,14},{11,13},{12,14},{13,14},{13,13}};
char score7[7][2] = {{9,12},{9,13},{9,14},{10,14},{11,14},{12,14},{13,14}};
char score8[13][2] = {{9,12},{9,13},{9,14},{10,12},{11,12},{12,12},{13,12},{13,13},{13,14},{12,14},{11,14},{10,14},{11,13}};
char score9[12][2] = {{9,12},{9,13},{9,14},{10,12},{11,12},{13,12},{13,13},{13,14},{12,14},{11,14},{10,14},{11,13}};
int score_length[10] = {12,5,11,11,9,11,10,7,13,12};
int nameMap[][2]={
{1,1},{1,2},{1,3},{1,4},{1,5},{1,6},{1,7},{1,8},{1,17},{1,18},{1,19},{1,20},{1,21},{1,22},{1,23},{1,24},{1,33},{1,34},{1,35},{1,36},{1,37},{1,38},{1,39},{1,40},{2,1},{2,2},{2,3},{2,4},{2,5},{2,6},{2,7},{2,8},{2,17},{2,18},{2,19},{2,20},{2,21},{2,22},{2,23},{2,24},{2,33},{2,34},{2,35},{2,36},{2,37},{2,38},{2,39},{2,40},{3,7},{3,8},{3,17},{3,18},{3,23},{3,24},{3,33},{3,34},{3,39},{3,40},{4,7},{4,8},{4,17},{4,18},{4,23},{4,24},{4,33},{4,34},{4,39},{4,40},{5,5},{5,6},{5,17},{5,18},{5,19},{5,20},{5,21},{5,22},{5,23},{5,24},{5,33},{5,34},{5,35},{5,36},{5,37},{5,38},{5,39},{5,40},{6,5},{6,6},{6,17},{6,18},{6,19},{6,20},{6,21},{6,22},{6,23},{6,24},{6,33},{6,34},{6,35},{6,36},{6,37},{6,38},{6,39},{6,40},{7,3},{7,4},{7,17},{7,18},{7,19},{7,20},{7,33},{7,34},{7,35},{7,36},{8,3},{8,4},{8,17},{8,18},{8,19},{8,20},{8,33},{8,34},{8,35},{8,36},{9,1},{9,2},{9,3},{9,4},{9,5},{9,6},{9,7},{9,8},{9,17},{9,18},{9,21},{9,22},{9,23},{9,24},{9,33},{9,34},{9,37},{9,38},{9,39},{9,40},{10,1},{10,2},{10,3},{10,4},{10,5},{10,6},{10,7},{10,8},{10,17},{10,18},{10,21},{10,22},{10,23},{10,24},{10,33},{10,34},{10,37},{10,38},{10,39},{10,40},{11,1},{11,2},{11,15},{11,16},{11,17},{11,18},{11,31},{11,32},{11,33},{11,34},{11,35},{11,36},{11,37},{11,38},{11,39},{11,40},{12,1},{12,2},{12,15},{12,16},{12,17},{12,18},{12,31},{12,32},{12,33},{12,34},{12,35},{12,36},{12,37},{12,38},{12,39},{12,40},{13,2},{13,3},{13,8},{13,9},{13,14},{13,15},{13,18},{13,19},{13,24},{13,25},{13,30},{13,31},{13,36},{13,37},{14,2},{14,3},{14,8},{14,9},{14,14},{14,15},{14,18},{14,19},{14,24},{14,25},{14,30},{14,31},{14,36},{14,37},{15,3},{15,4},{15,7},{15,8},{15,9},{15,10},{15,13},{15,14},{15,19},{15,20},{15,23},{15,24},{15,25},{15,26},{15,29},{15,30},{15,36},{15,37},{16,3},{16,4},{16,7},{16,8},{16,9},{16,10},{16,13},{16,14},{16,19},{16,20},{16,23},{16,24},{16,25},{16,26},{16,29},{16,30},{16,36},{16,37},{17,4},{17,5},{17,6},{17,7},{17,10},{17,11},{17,12},{17,13},{17,20},{17,21},{17,22},{17,23},{17,26},{17,27},{17,28},{17,29},{17,36},{17,37},{18,4},{18,5},{18,6},{18,7},{18,10},{18,11},{18,12},{18,13},{18,20},{18,21},{18,22},{18,23},{18,26},{18,27},{18,28},{18,29},{18,36},{18,37},{19,5},{19,6},{19,11},{19,12},{19,21},{19,22},{19,27},{19,28},{19,34},{19,35},{19,36},{19,37},{20,5},{20,6},{20,11},{20,12},{20,21},{20,22},{20,27},{20,28},{20,34},{20,35},{20,36},{20,37},{21,1},{21,2},{21,3},{21,4},{21,7},{21,8},{21,9},{21,10},{21,15},{21,16},{21,17},{21,18},{21,19},{21,20},{21,25},{21,26},{21,27},{21,28},{21,29},{21,30},{21,35},{21,36},{21,37},{21,38},{21,39},{21,40},{22,1},{22,2},{22,3},{22,4},{22,7},{22,8},{22,9},{22,10},{22,15},{22,16},{22,17},{22,18},{22,19},{22,20},{22,25},{22,26},{22,27},{22,28},{22,29},{22,30},{22,35},{22,36},{22,37},{22,38},{22,39},{22,40},{23,1},{23,2},{23,4},{23,5},{23,6},{23,7},{23,9},{23,10},{23,15},{23,16},{23,19},{23,20},{23,25},{23,26},{23,29},{23,30},{23,35},{23,36},{24,1},{24,2},{24,4},{24,5},{24,6},{24,7},{24,9},{24,10},{24,15},{24,16},{24,19},{24,20},{24,25},{24,26},{24,29},{24,30},{24,35},{24,36},{25,1},{25,2},{25,5},{25,6},{25,9},{25,10},{25,15},{25,16},{25,17},{25,18},{25,19},{25,20},{25,25},{25,26},{25,27},{25,28},{25,29},{25,30},{25,35},{25,36},{25,37},{25,38},{25,39},{25,40},{26,1},{26,2},{26,5},{26,6},{26,9},{26,10},{26,15},{26,16},{26,17},{26,18},{26,19},{26,20},{26,25},{26,26},{26,27},{26,28},{26,29},{26,30},{26,35},{26,36},{26,37},{26,38},{26,39},{26,40},{27,1},{27,2},{27,9},{27,10},{27,15},{27,16},{27,19},{27,20},{27,25},{27,26},{27,28},{27,29},{27,39},{27,40},{28,1},{28,2},{28,9},{28,10},{28,15},{28,16},{28,19},{28,20},{28,25},{28,26},{28,28},{28,29},{28,39},{28,40},{29,1},{29,2},{29,9},{29,10},{29,15},{29,16},{29,19},{29,20},{29,25},{29,26},{29,29},{29,30},{29,35},{29,36},{29,37},{29,38},{29,39},{29,40},{30,1},{30,2},{30,9},{30,10},{30,15},{30,16},{30,19},{30,20},{30,25},{30,26},{30,29},{30,30},{30,35},{30,36},{30,37},{30,38},{30,39},{30,40}
};

int foodnum = 0;			//食物数量
enum direc{ left, right, up, down };
int movedirection = right;       //蛇头方向
void My_ISR()__attribute__((interrupt_handler));
void Button_Handler();    //按键的中断控制函数
void GameOver_Show();//展示结束界面
void TimerCounterHandler() ;
void scan();
void makescore_ing();//ing显示得分
int button_status;        //按键状态
int sw_status;           //开关状态
int x,y;                 //当前下落图形的坐标
int x_next=24,y_next=4;       //
int num=0;                 //方块索引
void create_square();//生成方块
void clearLines(); //清除整行
void checkover();//判断游戏是否结束
void makescore();//结束界面展示得分

void inimap() {
	//打印name
	//for (int i=0;i<520;i++){
	//	map[nameMap[i][1]-1][nameMap[i][0]-1]=NAME;
	//}
	//scan();
	for (ULONG i = 0; i < 100000 ; i++);
	//初始化墙壁和空格
	for (int i = 1; i < HEIGHT_2-1; i++) {
			for (int j = 0; j < WIDTH-1; j++) {
				//if(i!=HEIGHT_1)
				//{
					map[i][j] = SPACE;
			   // }
			}
		}
	for (int i = 0; i < HEIGHT_2; i++) {
		for (int j = 0; j < WIDTH; j++) {
			map[0][j] = WALL;
			map[HEIGHT_1][j]=WALL;
			map[i][WIDTH - 1] = WALL;
			map[HEIGHT_2 - 1][j] = WALL;
		}
	}
	for(int i=HEIGHT_1;i<HEIGHT_2;i++){
		map[i][0]=WALL;
		map[i][7]=WALL;
	}
	scan();
	create_square();
	makescore_ing();
}

void create_square(){
	srand(a++);
	if(num==0)
		{
		shapeIndex = (rand() % 19) + 1;
		num=1;
		}
	else
	{
		shapeIndex=shapeIndex_next;

	for (int i = 0;i <4;i++){
		map[x_next + shapes[shapeIndex_next][i].shape_x ][y_next + shapes[shapeIndex_next][i].shape_y ] =SPACE;
		}
	}
	shapeIndex_next = (rand() % 19) + 1;
	x=9,y=2;
	for (int i = 0;i <4;i++){
		map[x + shapes[shapeIndex][i].shape_x][y + shapes[shapeIndex][i].shape_y] = move_square;
		map[x_next + shapes[shapeIndex_next][i].shape_x][y_next + shapes[shapeIndex_next][i].shape_y] = fixed_square;
	}
	//增加判断是否直接生成在fixed_square上置check=0
}



void scan() {
	int i, j;
	for (i = 0; i < HEIGHT_2; i++) {
		for (j = 0; j < WIDTH; j++) {
			//if(map[i][j] == lastmap[i][j])
				//continue;
			//lastmap[i][j] == map[i][j];
			if (map[i][j] == WALL) XTft_FillScreen(&TftInstance, i * ADD_NUM, j * ADD_NUM, (i + 1) * ADD_NUM - 1, (j + 1) * ADD_NUM - 1, WALL_COLOR);            //输出墙壁
			if (map[i][j] == SPACE) XTft_FillScreen(&TftInstance, i * ADD_NUM, j * ADD_NUM, (i + 1) * ADD_NUM - 1, (j + 1) * ADD_NUM - 1, SPACE_COLOR);     //输出空格
			if (map[i][j] == move_square)   XTft_FillScreen(&TftInstance, i * ADD_NUM, j * ADD_NUM, (i + 1) * ADD_NUM - 1, (j + 1) * ADD_NUM - 1, BODY_COLOR);      //可移动的方块
			if (map[i][j] == fixed_square)   XTft_FillScreen(&TftInstance, i * ADD_NUM, j * ADD_NUM, (i + 1) * ADD_NUM - 1, (j + 1) * ADD_NUM - 1, FOOD_COLOR);
		}
	}
	//memcpy(lastmap, map, sizeof(map));
}

void chageShape()//图形变换
{
	int ts = shapeIndex;
	switch(ts) {
	case 0:
		ts++;
		break;
	case 1:
		ts = 0;
		break;
	case 2:
		break;
	case 3:
	case 4:
	case 5:
		ts++;
		break;
	case 6:
		ts = 3;
		break;
	case 7:
	case 8:
	case 9:
		ts++;
		break;
	case 10:
		ts = 7;
		break;
	case 11:
	case 12:
	case 13:
		ts++;
		break;
	case 14:
		ts = 11;
		break;
	case 15:
		ts++;
		break;
	case 16:
		ts = 15;
		break;
	case 17:
		ts++;
		break;
	case 18:
		ts = 17;
		break;
	}
	//ts是假设变形后的图形值
	//判断假设的图形是否发生碰撞
	for(int i = 0;i < 4;i++) {
		int cx = x+shapes[ts][i].shape_x;
		int cy = y+shapes[ts][i].shape_y;
		if (cx < 1 || cx > HEIGHT_1-2  || cy > WIDTH-2 || map[cx][cy] == fixed_square || map[cx][cy] == WALL) {
			return;
		}
	}
	for(int i=0;i<4;i++){
		map[x+shapes[shapeIndex][i].shape_x][y+shapes[shapeIndex][i].shape_y]= SPACE;
	}
	for(int i=0;i<4;i++){
			map[x+shapes[ts][i].shape_x][y+shapes[ts][i].shape_y]= move_square;
		}
	shapeIndex = ts;
}

void move_left(){
	for (int i = 0; i < 4; i++) {
        if(map[x + shapes[shapeIndex][i].shape_x - 1][y + shapes[shapeIndex][i].shape_y ] == WALL
        || map[x + shapes[shapeIndex][i].shape_x - 1][y + shapes[shapeIndex][i].shape_y ] == fixed_square) {
            return;
        }
    }
	for (int i = 0; i < 4; i++)
	{
		int cx = x+shapes[shapeIndex][i].shape_x;
		int cy = y+shapes[shapeIndex][i].shape_y;
		map[cx][cy] = SPACE;
	}
    x--;
	for (int i = 0; i < 4; i++)
	{
		int cx = x+shapes[shapeIndex][i].shape_x;
		int cy = y+shapes[shapeIndex][i].shape_y;
		map[cx][cy] = move_square;
	}
}

void move_right(){
	for (int i = 0; i < 4; i++) {
        if(map[x + shapes[shapeIndex][i].shape_x + 1][y + shapes[shapeIndex][i].shape_y ] == WALL
        || map[x + shapes[shapeIndex][i].shape_x + 1][y + shapes[shapeIndex][i].shape_y ] == fixed_square) {
            return;
        }
    }
	for (int i = 0; i < 4; i++)
	{
		int cx = x+shapes[shapeIndex][i].shape_x;
		int cy = y+shapes[shapeIndex][i].shape_y;
		map[cx][cy] = SPACE;
	}
    x++;
	for (int i = 0; i < 4; i++)
	{
		int cx = x+shapes[shapeIndex][i].shape_x;
		int cy = y+shapes[shapeIndex][i].shape_y;
		map[cx][cy] = move_square;
	}
}

void speed(){
	for (int i = 0; i < 4; i++) {
	        if(map[x + shapes[shapeIndex][i].shape_x + 1][y + shapes[shapeIndex][i].shape_y ] == WALL
	        || map[x + shapes[shapeIndex][i].shape_x + 1][y + shapes[shapeIndex][i].shape_y ] == fixed_square) {
	            return;
	        }
	    }
	Xil_Out32(XPAR_AXI_TIMER_0_BASEADDR+XTC_TLR_OFFSET,(RESET_VALUE+2)/speed_down-2);
}
void clearLines(){
	int i, j;
	for (j = WIDTH-1; j >=1; j--)
	{
        int line_completed = 1; // 行是否已完成标志位，初始为已完成
		for (i = 1; i < HEIGHT_1; i++)
		{
			if(map[i][j] != fixed_square)
			{ line_completed = 0; // 只要有一个方块不是固定方块，表示该行未完成
            	break;
			}
		}

	        if (line_completed ) { // 如果行已完成
	            score++; // 已完成行数加一
	                for (int m = 1; m < HEIGHT_1; m++)
	                {
	           		 map[m][j] = SPACE;
	                }
	            // 将上方的所有行往下移动一行
	            for (int k = j; k > 1; k--) {
	                for (int n = 1; n < HEIGHT_1; n++) {
	                    map[n][k] = map[n][k-1];
	                }
	            }
	        }
	    }
}

void checkover(){//判断游戏是否结束
	for(int j=1;j>=0;j--){
		for(int i=1;i<HEIGHT_1 - 1;i++){
			if(map[i][j] == fixed_square){
				check=0;
				return;
			}
		}
	}
	check=1;
	return;
}

void GameOver_Show()
{
	int i, j;
	for (j = 120; j < 300; j++){
		for (i = 220; i < 460; i++){
			XTft_SetPixel(&TftInstance, i, j, gImage_gameover[(j - 120) * 240 + i - 220]);
		}
	}
    makescore();
    while(1){
	  Xil_Out16( XPAR_GPIO_1_BASEADDR+XGPIO_DATA_OFFSET,poscode[pos]);
	  Xil_Out16( XPAR_GPIO_1_BASEADDR+XGPIO_DATA2_OFFSET,segcode[pos]);
	  for(int i=0;i<1000;i++)
		  	 j=i;
	   pos++;
	  if(pos==8)
	     pos=0;
	}
}

void makescore()
{
	int temp;
	int first_num,second_num;
	first_num=(score)%10;
	temp=(score)/10;
	second_num=temp%10;
	segcode[7]=segtable[first_num];
	segcode[6]=segtable[second_num];
     segcode[0]=segtable[0];
	segcode[1]=0xc1;
	segcode[2]=segtable[14];
	segcode[3]=segtable[10];    //七段数码管显示over字样及所得分数
}

void makescore_ing()
{
	int i, j;
	int temp;
	int first_num,second_num;
	first_num=(score)%10;
	temp=(score)/10;
	second_num=temp%10;
	segcode[7]=segtable[first_num];
	segcode[6]=segtable[second_num];
     segcode[0]=segtable[1];
	segcode[1]=0xc8;
	segcode[2]=0x90;    //七段数码管显示over字样及所得分数
}

//void showscore(){
//	int score_len = score_length[score];
//	switch (score)
//		{
//		case 0:
//			for (int i = 0; i < score_len; i++) {
//				int x = score0[i][0];
//				int y = score0[i][1];
//			}
//			break;
//		case 1:
//			for (int i = 0; i < score_len; i++) {
//				int x = score0[i][0];
//				int y = score0[i][1];
//			}
//			break;
//		case 2:
//			for (int i = 0; i < score_len; i++) {
//				int x = score0[i][0];
//				int y = score0[i][1];
//			}
//			break;
//		case 3:
//			for (int i = 0; i < score_len; i++) {
//				int x = score0[i][0];
//				int y = score0[i][1];
//			}
//			break;
//		case 4:
//			for (int i = 0; i < score_len; i++) {
//				int x = score0[i][0];
//				int y = score0[i][1];
//			}
//			break;
//		case 5:
//			for (int i = 0; i < score_len; i++) {
//				int x = score0[i][0];
//				int y = score0[i][1];
//			}
//			break;
//		case 6:
//			for (int i = 0; i < score_len; i++) {
//				int x = score0[i][0];
//				int y = score0[i][1];
//			}
//			break;
//		case 7:
//			for (int i = 0; i < score_len; i++) {
//				int x = score0[i][0];
//				int y = score0[i][1];
//			}
//			break;
//		case 8:
//			for (int i = 0; i < score_len; i++) {
//				int x = score0[i][0];
//				int y = score0[i][1];
//			}
//			break;
//		case 9:
//			for (int i = 0; i < score_len; i++) {
//				int x = score0[i][0];
//				int y = score0[i][1];
//			}
//			break;
//		default:
//			break;
//		}
//}

int main()
{
	//中断初始化
    //Button
    Xil_Out32(XPAR_AXI_GPIO_0_BASEADDR+XGPIO_TRI_OFFSET,0xffff);//输入方向
    Xil_Out32(XPAR_AXI_GPIO_0_BASEADDR+XGPIO_IER_OFFSET,0x1);//使能1号引脚中断
    Xil_Out32(XPAR_AXI_GPIO_0_BASEADDR+XGPIO_GIE_OFFSET,0x80000000);//使能中断输出
    //INTC
    Xil_Out32(XPAR_AXI_INTC_0_BASEADDR+XIN_IER_OFFSET,0x3);//使能GPIO和Timer中断
    Xil_Out32(XPAR_AXI_INTC_0_BASEADDR+XIN_MER_OFFSET,0x3);//使能中断输出

    Xil_Out32(XPAR_AXI_GPIO_1_BASEADDR+XGPIO_TRI_OFFSET,0x00);
	Xil_Out32(XPAR_AXI_GPIO_1_BASEADDR+XGPIO_TRI2_OFFSET,0x00);

	//Xil_Out32(XPAR_AXI_GPIO_0_BASEADDR + XGPIO_TRI_OFFSET, 0Xffff);//设定开关为输入方式
	//Xil_Out32(XPAR_AXI_GPIO_0_BASEADDR + XGPIO_TRI_OFFSET, 0X1f);//设定BUTTON为输入方式
	//Xil_Out32(XPAR_AXI_GPIO_0_BASEADDR + XGPIO_IER_OFFSET, XGPIO_IR_CH1_MASK);//通道1允许中断
	//Xil_Out32(XPAR_AXI_GPIO_0_BASEADDR + XGPIO_GIE_OFFSET, XGPIO_GIE_GINTR_ENABLE_MASK);//允许GPIO中断输出
	//Xil_Out32(XPAR_AXI_INTC_0_BASEADDR + XIN_IER_OFFSET, XPAR_AXI_GPIO_0_IP2INTC_IRPT_MASK);//对中断控制器进行中断源使能
	//Xil_Out32(XPAR_AXI_INTC_0_BASEADDR + XIN_MER_OFFSET, XIN_INT_MASTER_ENABLE_MASK | XIN_INT_HARDWARE_ENABLE_MASK);
    //Xil_Out32(XPAR_AXI_GPIO_1_BASEADDR+XGPIO_TRI_OFFSET,0x00);
	//Xil_Out32(XPAR_AXI_GPIO_1_BASEADDR+XGPIO_TRI2_OFFSET,0x00);

	Xil_Out32(XPAR_AXI_GPIO_0_BASEADDR+XGPIO_TRI2_OFFSET,0x0);
	Xil_Out32(XPAR_AXI_TIMER_0_BASEADDR+XTC_TCSR_OFFSET,
	    		Xil_In32(XPAR_AXI_TIMER_0_BASEADDR+XTC_TCSR_OFFSET)&~XTC_CSR_ENABLE_TMR_MASK);
	Xil_Out32(XPAR_AXI_TIMER_0_BASEADDR+XTC_TLR_OFFSET,RESET_VALUE);
	Xil_Out32(XPAR_AXI_TIMER_0_BASEADDR+XTC_TCSR_OFFSET,
	    	Xil_In32(XPAR_AXI_TIMER_0_BASEADDR+XTC_TCSR_OFFSET)|XTC_CSR_LOAD_MASK);
	Xil_Out32(XPAR_AXI_TIMER_0_BASEADDR+XTC_TCSR_OFFSET,
	    	(Xil_In32(XPAR_AXI_TIMER_0_BASEADDR+XTC_TCSR_OFFSET)&~XTC_CSR_LOAD_MASK)\
			|XTC_CSR_ENABLE_TMR_MASK|XTC_CSR_AUTO_RELOAD_MASK|XTC_CSR_ENABLE_INT_MASK|XTC_CSR_DOWN_COUNT_MASK);

	//Xil_Out32(XPAR_AXI_INTC_0_BASEADDR+XIN_IER_OFFSET,XPAR_AXI_TIMER_0_INTERRUPT_MASK);
	// Xil_Out32(XPAR_AXI_INTC_0_BASEADDR+XIN_MER_OFFSET,XIN_INT_MASTER_ENABLE_MASK|XIN_INT_HARDWARE_ENABLE_MASK);

	TftConfigPtr = XTft_LookupConfig(TFT_DEVICE_ID);
	XTft_CfgInitialize(&TftInstance, TftConfigPtr, TftConfigPtr->BaseAddress); //初始化tft
	XTft_SetFrameBaseAddr(&TftInstance, TFT_FRAME_BASEADDR0);//设置tft的存储基地址
	microblaze_enable_interrupts();//允许微处理器接受中断
	//Xil_Out16( XPAR_GPIO_1_BASEADDR + XGPIO_DATA_OFFSET, 0xffff);
	XTft_ClearScreen(&TftInstance);
	inimap();
	int i,j;
    while(1){
	  Xil_Out16( XPAR_GPIO_1_BASEADDR+XGPIO_DATA_OFFSET,poscode[pos]);
	  Xil_Out16( XPAR_GPIO_1_BASEADDR+XGPIO_DATA2_OFFSET,segcode[pos]);
	  for( i=0;i<1000;i++)
		  	 j=i;
	   pos++;
	  if(pos==8)
	     pos=0;
	}
	return 0;
}

void My_ISR()
{
	  int status;
	  status = Xil_In32(XPAR_INTC_0_BASEADDR + XIN_ISR_OFFSET);
	  if ((status & XPAR_AXI_TIMER_0_INTERRUPT_MASK) == XPAR_AXI_TIMER_0_INTERRUPT_MASK)
	    TimerCounterHandler();
	  //if ((status & XPAR_AXI_GPIO_0_IP2INTC_IRPT_MASK) == XPAR_AXI_GPIO_0_IP2INTC_IRPT_MASK)
	   // swHandler();
	  if ((status & XPAR_AXI_GPIO_0_IP2INTC_IRPT_MASK) == XPAR_AXI_GPIO_0_IP2INTC_IRPT_MASK)
		  Button_Handler();
	  Xil_Out32(XPAR_INTC_0_BASEADDR + XIN_IAR_OFFSET, status);
}

void TimerCounterHandler()
{
	int state=0;//是否能下降
	for (int i = 0;i <4;i++){
		if(map[x + shapes[shapeIndex][i].shape_x][y + 1 + shapes[shapeIndex][i].shape_y] ==WALL||map[x + shapes[shapeIndex][i].shape_x][y + 1 + shapes[shapeIndex][i].shape_y] ==fixed_square)
			 {
			 state=0;
			 break;
			 }
		else state=1;
	}

    if(state)
   {
	  for (int i = 0;i <4;i++){
		 map[x + shapes[shapeIndex][i].shape_x][y + shapes[shapeIndex][i].shape_y] = SPACE;
	  }
	  y=y+1;
	  for (int i = 0;i <4;i++){
		     map[x + shapes[shapeIndex][i].shape_x][y + shapes[shapeIndex][i].shape_y] = move_square;
	  }
   }
	  else
	  {
		for (int i = 0;i <4;i++)
				map[x + shapes[shapeIndex][i].shape_x][y  + shapes[shapeIndex][i].shape_y] =fixed_square;
		  clearLines();
		  checkover();
		  if(check){
			create_square();
		  }
		  else{
			scan();
			GameOver_Show();
		  }

	  }
scan();
Xil_Out32(XPAR_AXI_TIMER_0_BASEADDR+XTC_TCSR_OFFSET,Xil_In32(XPAR_AXI_TIMER_0_BASEADDR+XTC_TCSR_OFFSET));
}


void Button_Handler(){
	button_status = Xil_In8(XPAR_GPIO_0_BASEADDR + XGPIO_DATA_OFFSET) & 0x1f;  //读取按键的状态值
	if (button_status == Btn_UP)
		{
		chageShape();
		scan();
		}
	if ((button_status == Btn_DOWN))
	{
		    speed();
			scan();
			}

	if (button_status == Btn_LEFT)
		{
		move_left();
		scan();
		}
	if ((button_status == Btn_RIGHT))
		{
		move_right();
		scan();
		}
	if((button_status==0x1)&&(speed_down<speed_max)){
		speed_down++;
		Xil_Out32(XPAR_AXI_TIMER_0_BASEADDR+XTC_TLR_OFFSET,(RESET_VALUE+2)/speed_down-2);
	}
	else if((button_status==0x1)&&speed_down==speed_max){
		speed_down=1;
		Xil_Out32(XPAR_AXI_TIMER_0_BASEADDR+XTC_TLR_OFFSET,(RESET_VALUE+2)/speed_down-2);}

	Xil_Out32(XPAR_GPIO_0_BASEADDR + XGPIO_ISR_OFFSET, XGPIO_IR_MASK);//写IPISR，清除中断
	Xil_Out32(XPAR_AXI_INTC_0_BASEADDR + XIN_IAR_OFFSET, 0x2);//写INTC_IAR，清除中断状态
}
