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
#include "color.h"			//方块贴图的数组矩阵
/*-----------------------------函数定义------------------------------------*/
void My_ISR()__attribute__((interrupt_handler));
void Button_Handler();    			//按键的中断控制函数
void TimerCounterHandler();			//定时器中断控制函数

void create_square();				//生成方块
void scan();						//扫掠界面-对应方块上色
void square_Show(int m,int n);		//方块贴图
void clear_single_line(int row);	//消去单独的一行，传入行号
void clearLines(); 					//清除整行

void caculate();					//计算分数位次
void showscore_first();				//展示个位分数
void showscore_second();			//展示十位分数
void screen_first();				//清除个位分数
void screen_second();				//清除十位分数
void makescore_ing();				//ing 状态显示得分
void makescore_stop();				//stop状态显示得分
void makescore();					//结束界面展示得分

void checkover();					//判断游戏是否结束
void GameOver_Show();				//展示结束界面

void push();						//入栈操作
void clearStack();					//清栈操作
int pop();							//出栈操作
int peek();							//获取栈顶元素
int isEmpty();						//判断栈是否为空
int isFull();						//判断栈是否已满
/*-----------------------------界面定义------------------------------------*/
#define START_X 0         	//屏幕起始x坐标
#define START_Y 0          	//屏幕起始y坐标
#define ADD_NUM 18          //像素块大小
#define TFT_BASEADDR XPAR_AXI_TFT_0_BASEADDR	//基地址的宏定义
#define TFT_FRAME_BASEADDR XPAR_MIG7SERIES_0_BASEADDR
#define RESET_VALUE 100000000-2					//定时器周期1s

/*-----------------------------按钮定义------------------------------------*/
#define TFT_DEVICE_ID XPAR_TFT_0_DEVICE_ID
#define TFT_FRAME_BASEADDR0 0X86000000
#define TFT_FRAME_BASEADDR1 0X84000000
#define Btn_UP 0x2
#define Btn_DOWN 0x10
#define Btn_LEFT 0x4
#define Btn_RIGHT 0x8
#define Btn_CENTER 0x1

/*-----------------------------类型定义------------------------------------*/
enum thing
{
	NAME,			//游戏名
	WALL,			//墙壁
	SPACE,			//空白
    move_square,	//可移动方块
	fixed_square,	//已固定方块
	next_square,	//下一个方块
	display_score	//得分显示
};

/*-----------------------------方块颜色------------------------------------*/
#define color_1 0x00264653      //R:038 G:070 B:083 色
#define color_2 0x00e66f51     	//R:230 G:111 B:081	色
#define color_3 0x002a9d8c		//R:042 G:157 B:140	色
#define color_4 0x008ab07d      //R:138 G:176 B:125	色
#define color_5 0x00e9c46b      //R:233 G:196 B:107	色
#define color_6 0x00f3a261     	//R:243 G:162 B:097	色
#define color_score 0x00ff7271		//R:040 G:114 B:113  得分颜色
#define color_white 0xffffffff      //R:255 G:255 B:255 白色墙壁


/*-----------------------------方块结构------------------------------------*/
struct Point{
	int shape_x;
	int shape_y;
    u32 color;  // 添加一个表示颜色的成员
} ;

struct Point shapes[19][4] = {
    {{0, 0, color_1},{-1, 0, color_1},{ 1, 0, color_1},{ 2, 0, color_1}},      //横条
    {{0, 0, color_1},{ 0,-1, color_1},{ 0, 1, color_1},{ 0, 2, color_1}},      //竖条
	{{0, 0, color_2},{-1,-1, color_2},{-1, 0, color_2},{ 0,-1, color_2}},      //方块
	{{0, 0, color_3},{ 0,-1, color_3},{ 0,-2, color_3},{ 1, 0, color_3}},      //正	L1
	{{0, 0, color_3},{ 0, 1, color_3},{ 1, 0, color_3},{ 2, 0, color_3}},      //正	L2
	{{0, 0, color_3},{-1, 0, color_3},{ 0, 1, color_3},{ 0, 2, color_3}},      //正	L3
    {{0, 0, color_3},{ 0,-1, color_3},{-1, 0, color_3},{-2, 0, color_3}},      //正	L4
    {{0, 0, color_4},{-1, 0, color_4},{ 0,-1, color_4},{ 0,-2, color_4}},      //反	L1
    {{0, 0, color_4},{ 0,-1, color_4},{ 1, 0, color_4},{ 2, 0, color_4}},      //反	L2
    {{0, 0, color_4},{ 1, 0, color_4},{ 0, 1, color_4},{ 0, 2, color_4}},      //反	L3
    {{0, 0, color_4},{-1, 0, color_4},{-2, 0, color_4},{ 0, 1, color_4}},      //反	L4
    {{0, 0, color_5},{-1, 0, color_5},{ 1, 0, color_5},{ 0,-1, color_5}},      //T 1
    {{0, 0, color_5},{ 0, 1, color_5},{ 0,-1, color_5},{ 1, 0, color_5}},      //T 2
    {{0, 0, color_5},{-1, 0, color_5},{ 1, 0, color_5},{ 0, 1, color_5}},      //T 3
    {{0, 0, color_5},{-1, 0, color_5},{ 0,-1, color_5},{ 0, 1, color_5}},      //T 4
    {{0, 0, color_6},{ 1, 0, color_6},{ 0,-1, color_6},{-1,-1, color_6}}, 	   //正	Z1
    {{0, 0, color_6},{ 1,-1, color_6},{ 0, 1, color_6},{ 1, 0, color_6}},      //正	Z2
    {{0, 0, color_6},{ 1,-1, color_6},{-1, 0, color_6},{ 0,-1, color_6}},      //反	Z1
	{{0, 0, color_6},{-1,-1, color_6},{-1, 0, color_6},{ 0, 1, color_6}}       //反	Z2
};

/*-----------------------------墙壁位置------------------------------------*/
const int HEIGHT_1 = 19;	//游戏区x轴长度
const int HEIGHT_2 = 30;	//显示区x轴长度
const int WIDTH_1 = 7;		//显示区y轴分割
const int WIDTH_2 = 19;		//游戏区y轴长度

/*-----------------------------参数定义------------------------------------*/
int map[30][23];			//创建地图大小
int map_color[30][23];		//存储不同方块颜色
int nameMap[][2]={{3,3},{3,4},{3,5},{3,6},{3,7},{1,3},{2,3},{4,3},{5,3},
				  {7,4},{8,3},{9,4},{9,5},{7,5},{8,5},{7,7},{7,6},{8,7},{9,7},{7,3},{9,3},
				  {10,4},{11,4},{12,4},{13,4},{14,4},{12,3},{12,5},{12,6},{12,7},{13,7}//tetris
};							//显示游戏名
int X,Y;                 	//生成下落方块的坐标
int x_next=24,y_next=3;     //后续方块显示位置
int num=0;                 	//方块索引
int shapeIndex=3; 			//方块索引，取值为1-19
int shapeIndex_next;		//后续方块索引，取值为1-19

int button_status;  //按键状态
int score=59;		//初始得分
int first_num=0;	//得分个位
int second_num=0;	//得分十位

char speed_down=1;	//下降速度等级
char speed_max=3;	//最大速度等级

static XTft TftInstance;     //实例化tft
XTft_Config* TftConfigPtr;

int a=2;				//随机种子生成随机序列
int begin=1;			//判断游戏是否暂停（1为进行，2为暂停）
int check=1;			//判断游戏是否结束（1为进行，2为结束）

/*-----------------------------得分显示------------------------------------*/
int pos=0;//七段数码管位码
char segtable[16]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,
		0x82,0xf8,0x80,0x98,0x88,0x83,0xc6,0xa1,0x86,0x8e};//七段数码管段码
char poscode[8]={0x7f,0xbf,0xdf,0xef,0xf7,0xfb,0xfd,0xfe};
char segcode[8]={0xff,0xff,0xff,0xff,0xff,0xff,0xc0,0xc0};
//数字1、2、3、4、5、6、7、8、9
char score0[12][2] = {{21,11},{21,12},{21,13},{21,14},{21,15},{22,11},{22,15},{23,11},{23,12},{23,13},{23,14},{23,15}};//0
char score1[ 5][2] = {{23,11},{23,12},{23,13},{23,14},{23,15}};//1
char score2[11][2] = {{21,11},{21,13},{21,14},{21,15},{22,11},{22,13},{22,15},{23,11},{23,12},{23,13},{23,15}};//2
char score3[11][2] = {{21,11},{21,13},{21,15},{22,11},{22,13},{22,15},{23,11},{23,12},{23,13},{23,14},{23,15}};//3
char score4[ 9][2] = {{21,11},{21,12},{21,13},{22,13},{23,11},{23,12},{23,13},{23,14},{23,15}};//4
char score5[11][2] = {{21,11},{21,12},{21,13},{21,15},{22,11},{22,13},{22,15},{23,11},{23,13},{23,14},{23,15}};//5
char score6[12][2] = {{21,11},{21,12},{21,13},{21,14},{21,15},{22,11},{22,13},{22,15},{23,11},{23,13},{23,14},{23,15}};//6
char score7[ 7][2] = {{21,11},{22,11},{23,11},{23,12},{23,13},{23,14},{23,15}};//7
char score8[13][2] = {{21,11},{21,12},{21,13},{21,14},{21,15},{22,11},{22,13},{22,15},{23,11},{23,12},{23,13},{23,14},{23,15}};//8
char score9[12][2] = {{21,11},{21,12},{21,13},{21,15},{22,11},{22,13},{22,15},{23,11},{23,12},{23,13},{23,14},{23,15}};//9
int score_length[10] = {12,5,11,11,9,11,12,7,13,12};

/*-----------------------------栈的定义------------------------------------*/
#define MAX_SIZE 5 			// 栈的最大容量
// 定义栈的结构体
typedef struct {
    int top;      			// 栈顶索引
    int array[MAX_SIZE];  	// 存储栈元素的数组
} Stack;

// 创建一个新栈
Stack* createStack() {
    Stack* stack = (Stack*)malloc(sizeof(Stack));
    if (!stack) {
        printf("内存分配失败\n");
        return NULL;
    }
    stack->top = -1; 		// 初始化栈顶索引为-1
    return stack;
}

// 判断栈是否为空
int isEmpty(Stack* stack) {
    return stack->top == -1;
}

// 判断栈是否已满
int isFull(Stack* stack) {
    return stack->top == MAX_SIZE - 1;
}

// 入栈操作
void push(Stack* stack, int item) {
    if (isFull(stack)) {
        printf("栈已满\n");
        return;
    }
    stack->array[++stack->top] = item;
    printf("%d 入栈\n", item);
}

// 出栈操作
int pop(Stack* stack) {		//栈为空，返回1；栈不为空，返回0
    if (isEmpty(stack)) {
        printf("栈为空\n");
        return -1;
    }
    printf("%d 出栈\n", stack->array[stack->top]);
    return stack->array[stack->top--];
}

// 获取栈顶元素
int peek(Stack* stack) {
    if (isEmpty(stack)) {
        printf("栈为空\n");
        return -1;
    }
    return stack->array[stack->top];
}
//清栈
void clearStack(Stack* stack) {
    while (!isEmpty(stack)) {
        pop(stack);
    }
}

Stack stack;				//全局栈

/*-----------------------------初始化------------------------------------*/

//初始化地图界面
void inimap() {
	for (int i = 0; i < HEIGHT_2; i++)
			for (int j = 0; j < WIDTH_2; j++)
					map[i][j] = SPACE;
	scan();
	int a,b=0;
	for (int i=0;i<31;i++){//打印name
		a=nameMap[i][0];
		b=nameMap[i][1];
		map[a][b]=NAME;
	}
	scan();
	//延迟显示游戏名
	Xil_Out32(XPAR_AXI_TIMER_0_BASEADDR+XTC_TCSR_OFFSET,Xil_In32(XPAR_AXI_TIMER_0_BASEADDR+XTC_TCSR_OFFSET)&0xff7f);//计数器暂停
	sleep(5);//等待
	Xil_Out32(XPAR_AXI_TIMER_0_BASEADDR+XTC_TCSR_OFFSET,Xil_In32(XPAR_AXI_TIMER_0_BASEADDR+XTC_TCSR_OFFSET)|0x80);//计数器启动

/*-----------------------------画地图------------------------------------*/
	//画背景
	for (int i = 1; i < HEIGHT_2-1; i++) {
			for (int j = 0; j < WIDTH_2-1; j++) {
					map[i][j] = SPACE;
					map_color[i][j]=color_white;
			}
		}
	//画竖直墙壁与底部墙壁
	for (int i = 0; i < HEIGHT_2; i++) {
		for (int j = 0; j < WIDTH_2; j++) {
			map[0][j] = WALL;
			map[HEIGHT_1][j]=	WALL;
			map[HEIGHT_2-1][j]= WALL;
			map[i][WIDTH_2-1] = WALL;
		}
	}
	//画显示区水平墙壁
	for(int i=HEIGHT_1;i<HEIGHT_2;i++){
		map[i][0]=WALL;
		map[i][WIDTH_1]=WALL;
	}
	//为方块染色
	scan();
	//生成新方块
	create_square();
	//数码管显示得分
	makescore_ing();
	//显示区显示得分
	showscore_first();
	showscore_second();
}

//生成方块
void create_square(){
	//设置方块生成位置
	X=9,Y=-2;
	srand(a++);
	//第一次生成方块
	if(num==0)
		{
		shapeIndex = rand() % 19 ;
		num=1;
		}
	//第n次生成方块
	else
	{
		shapeIndex=shapeIndex_next;

	for (int i = 0;i <4;i++)
		map[x_next + shapes[shapeIndex_next][i].shape_x ][y_next + shapes[shapeIndex_next][i].shape_y ] =SPACE;
	}
	 shapeIndex_next = rand() % 19 ;

	for (int i = 0;i <4;i++){
		map[X + shapes[shapeIndex][i].shape_x][Y + shapes[shapeIndex][i].shape_y] = move_square;
		map[x_next + shapes[shapeIndex_next][i].shape_x][y_next + shapes[shapeIndex_next][i].shape_y] = next_square;
	    map_color[x_next + shapes[shapeIndex_next][i].shape_x]
				  [y_next + shapes[shapeIndex_next][i].shape_y] = shapes[shapeIndex_next][i].color;
	}
}

//扫掠染色方块
void scan() {
	int i, j;
	for (i = 0; i < HEIGHT_2; i++) {
		for (j = 0; j < WIDTH_2; j++) {
			if (map[i][j] == WALL) square_Show(i,j);
			if (map[i][j] == NAME) XTft_FillScreen(&TftInstance, i * ADD_NUM, j * ADD_NUM, (i + 1) * ADD_NUM - 1, (j + 1) * ADD_NUM - 1, color_1);     								//输出游戏名
			if (map[i][j] == SPACE) XTft_FillScreen(&TftInstance, i * ADD_NUM, j * ADD_NUM, (i + 1) * ADD_NUM - 1, (j + 1) * ADD_NUM - 1, color_white);     							//输出空白
			if (map[i][j] == move_square)   XTft_FillScreen(&TftInstance, i * ADD_NUM, j * ADD_NUM, (i + 1) * ADD_NUM - 1, (j + 1) * ADD_NUM - 1, shapes[shapeIndex][0].color);     	//可移动的方块
			if (map[i][j] == fixed_square)   XTft_FillScreen(&TftInstance, i * ADD_NUM, j * ADD_NUM, (i + 1) * ADD_NUM - 1, (j + 1) * ADD_NUM - 1, map_color[i][j]);					//已固定的方块
			if (map[i][j] == next_square)   XTft_FillScreen(&TftInstance, i * ADD_NUM, j * ADD_NUM, (i + 1) * ADD_NUM - 1, (j + 1) * ADD_NUM - 1, shapes[shapeIndex_next][0].color);	//后续方块
			if (map[i][j] == display_score)   XTft_FillScreen(&TftInstance, i * ADD_NUM, j * ADD_NUM, (i + 1) * ADD_NUM - 1, (j + 1) * ADD_NUM - 1, color_score);						//输出得分
		}
	}
}

//为墙壁染色
void square_Show(int m,int n)
{
	int x,y;
	for (y = 18*n; y <18+18*n; y++){
		for (x = 18*m; x < 18+18*m; x++){
				XTft_SetPixel(&TftInstance, x, y, gImage_color[(y - 18*n) * 18 + x - 18*m]);
		}
	}
}

/*-----------------------------方块控制------------------------------------*/

//方块旋转
void chageShape()
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
		int cx = X+shapes[ts][i].shape_x;
		int cy = Y+shapes[ts][i].shape_y;
		if (cx < 1 || cx > HEIGHT_1-2  || cy > WIDTH_2-2 || map[cx][cy] == fixed_square || map[cx][cy] == WALL) {
			return;
		}
	}
	for(int i=0;i<4;i++){
		map[X+shapes[shapeIndex][i].shape_x][Y+shapes[shapeIndex][i].shape_y]= SPACE;
	}
	for(int i=0;i<4;i++){
			map[X+shapes[ts][i].shape_x][Y+shapes[ts][i].shape_y]= move_square;
		}
	shapeIndex = ts;
}

//方块左移
void move_left(){
	for (int i = 0; i < 4; i++) {
        if(map[X + shapes[shapeIndex][i].shape_x - 1][Y + shapes[shapeIndex][i].shape_y ] == WALL
        || map[X + shapes[shapeIndex][i].shape_x - 1][Y + shapes[shapeIndex][i].shape_y ] == fixed_square) {
            return;
        }
    }
	for (int i = 0; i < 4; i++)
	{
		int cx = X+shapes[shapeIndex][i].shape_x;
		int cy = Y+shapes[shapeIndex][i].shape_y;
		map[cx][cy] = SPACE;
	}
    X--;
	for (int i = 0; i < 4; i++)
	{
		int cx = X+shapes[shapeIndex][i].shape_x;
		int cy = Y+shapes[shapeIndex][i].shape_y;
		map[cx][cy] = move_square;
	}
}

//方块右移
void move_right(){
	for (int i = 0; i < 4; i++) {
        if(map[X + shapes[shapeIndex][i].shape_x + 1][Y + shapes[shapeIndex][i].shape_y ] == WALL
        || map[X + shapes[shapeIndex][i].shape_x + 1][Y + shapes[shapeIndex][i].shape_y ] == fixed_square) {
            return;
        }
    }
	for (int i = 0; i < 4; i++)
	{
		int cx = X+shapes[shapeIndex][i].shape_x;
		int cy = Y+shapes[shapeIndex][i].shape_y;
		map[cx][cy] = SPACE;
	}
    X++;
	for (int i = 0; i < 4; i++)
	{
		int cx = X+shapes[shapeIndex][i].shape_x;
		int cy = Y+shapes[shapeIndex][i].shape_y;
		map[cx][cy] = move_square;
	}
}

//消去单独的一行，传入行号
void clear_single_line(int row){
	score++; // 得分加一
	makescore_ing();
	caculate();
	showscore_first();
	showscore_second();
	for (int m = 1; m < HEIGHT_1; m++){
	     map[m][row] = SPACE;
	}
	// 将上方的所有行往下移动一行
	for (int k = row; k > 1; k--) {
	    for (int n = 1; n < HEIGHT_1; n++) {
	                map[n][k] = map[n][k-1];
	                map_color[n][k]=map_color[n][k-1];
	         }
	    }
}

void clearLines(){
	int i, j;
	for (j = WIDTH_2-1; j >=1; j--){
		for (i = 1; i < HEIGHT_1; i++){
			if(map[i][j] != fixed_square){
            	break;
			}
			if (i == HEIGHT_1 -1)
			{
				push(&stack,j);
			}
		}
	}
	if (isEmpty(&stack)){
		return;
	}else{
		while (!isEmpty(&stack))
		{
			int row=pop(&stack);
			clear_single_line(row);
		}
	}
}

/*-----------------------------分数处理------------------------------------*/

//位数计算
void caculate(){
	int temp;
		first_num=(score)%10;
		temp=(score)/10;
		second_num=temp%10;
}

//清除个位分数
void screen_first(){
	int i, j;
		for (i = HEIGHT_1+4; i<HEIGHT_2-1; i++)
		{
			for (j = WIDTH_1+1; j < WIDTH_2-1; j++)
			{
				map[i][j]=SPACE;
			}
		}
}

//清除十位分数
void screen_second(){
	int i, j;
		for (i = HEIGHT_1+1; i<HEIGHT_1+4; i++)
		{
			for (j = WIDTH_1+1; j < WIDTH_2-1; j++)
			{
				map[i][j]=SPACE;
			}
		}
}

//显示个位分数
void showscore_first(){
	int score_len = score_length[first_num];
	switch (first_num)
		{
		case 0:
			screen_first();
					for (int i = 0; i < score_len; i++) {
						int x = score0[i][0]+4;
						int y = score0[i][1]+1;
						map[x][y]=display_score;
					}
			break;
		case 1:
			screen_first();
					for (int i = 0; i < score_len; i++) {
						int x = score1[i][0]+4;
						int y = score1[i][1]+1;
						map[x][y]=display_score;
					}
					break;
		case 2:
			screen_first();
					for (int i = 0; i < score_len; i++) {
						int x = score2[i][0]+4;
						int y = score2[i][1]+1;
						map[x][y]=display_score;
					}
					break;
		case 3:
			screen_first();
					for (int i = 0; i < score_len; i++) {
						int x = score3[i][0]+4;
						int y = score3[i][1]+1;
						map[x][y]=display_score;
					}
					break;
		case 4:
			screen_first();
					for (int i = 0; i < score_len; i++) {
						int x = score4[i][0]+4;
						int y = score4[i][1]+1;
						map[x][y]=display_score;
					}
					break;
		case 5:
			screen_first();
					for (int i = 0; i < score_len; i++) {
						int x = score5[i][0]+4;
						int y = score5[i][1]+1;
						map[x][y]=display_score;
					}
					break;
		case 6:
			screen_first();
					for (int i = 0; i < score_len; i++) {
						int x = score6[i][0]+4;
						int y = score6[i][1]+1;
						map[x][y]=display_score;
					}
					break;
		case 7:
			screen_first();
					for (int i = 0; i < score_len; i++) {
						int x = score7[i][0]+4;
						int y = score7[i][1]+1;
						map[x][y]=display_score;
					}
					break;
		case 8:
			screen_first();
					for (int i = 0; i < score_len; i++) {
						int x = score8[i][0]+4;
						int y = score8[i][1]+1;
						map[x][y]=display_score;
					}
					break;
		case 9:
			screen_first();
					for (int i = 0; i < score_len; i++) {
						int x = score9[i][0]+4;
						int y = score9[i][1]+1;
						map[x][y]=display_score;
					}
					break;
		default:
					break;
		}
}

//显示十位分数
void showscore_second(){
	int score_len = score_length[second_num];
	switch (second_num)
		{
		case 0:
			screen_second();
					for (int i = 0; i < score_len; i++) {
						int x = score0[i][0];
						int y = score0[i][1]+1;
						map[x][y]=display_score;
					}
			break;
		case 1:
			screen_second();
					for (int i = 0; i < score_len; i++) {
						int x = score1[i][0];
						int y = score1[i][1]+1;
						map[x][y]=display_score;
					}
					break;
		case 2:
			screen_second();
					for (int i = 0; i < score_len; i++) {
						int x = score2[i][0];
						int y = score2[i][1]+1;
						map[x][y]=display_score;
					}
					break;
		case 3:
			screen_second();
					for (int i = 0; i < score_len; i++) {
						int x = score3[i][0];
						int y = score3[i][1]+1;
						map[x][y]=display_score;
					}
					break;
		case 4:
			screen_second();
					for (int i = 0; i < score_len; i++) {
						int x = score4[i][0];
						int y = score4[i][1]+1;
						map[x][y]=display_score;
					}
					break;
		case 5:
			screen_second();
					for (int i = 0; i < score_len; i++) {
						int x = score5[i][0];
						int y = score5[i][1]+1;
						map[x][y]=display_score;
					}
					break;
		case 6:
			screen_second();
					for (int i = 0; i < score_len; i++) {
						int x = score6[i][0];
						int y = score6[i][1]+1;
						map[x][y]=display_score;
					}
					break;
		case 7:
			screen_second();
					for (int i = 0; i < score_len; i++) {
						int x = score7[i][0];
						int y = score7[i][1]+1;
						map[x][y]=display_score;
					}
					break;
		case 8:
			screen_second();
					for (int i = 0; i < score_len; i++) {
						int x = score8[i][0];
						int y = score8[i][1]+1;
						map[x][y]=display_score;
					}
					break;
		case 9:
			screen_second();
					for (int i = 0; i < score_len; i++) {
						int x = score9[i][0];
						int y = score9[i][1]+1;
						map[x][y]=display_score;
					}
					break;
		default:
					break;
		}
}

/*-----------------------------数码管显示------------------------------------*/


void makescore()
{
	caculate();
	segcode[7]=segtable[first_num];
	segcode[6]=segtable[second_num];
    segcode[0]=segtable[0];
	segcode[1]=0xc1;
	segcode[2]=segtable[14];
	segcode[3]=segtable[10];    //七段数码管显示over字样及所得分数
}

//游戏进行中显示得分
void makescore_ing()
{
	int temp;
	int first_num,second_num;
	first_num=(score)%10;
	temp=(score)/10;
	second_num=temp%10;
	segcode[7]=segtable[first_num];
	segcode[6]=segtable[second_num];
    segcode[0]=segtable[1];
	segcode[1]=0xc8;
	segcode[2]=0x90;
	segcode[3]=0xff;//七段数码管显示ing字样及所得分数
}

//游戏暂停时显示得分
void makescore_stop()
{
	int temp;
	int first_num,second_num;
	first_num=(score)%10;
	temp=(score)/10;
	second_num=temp%10;
	segcode[7]=segtable[first_num];
	segcode[6]=segtable[second_num];
    segcode[0]=0x92;
	segcode[1]=0x87;
	segcode[2]=0xc0;
	segcode[3]=0x8c;//七段数码管显示stop字样及所得分数
}

/*-----------------------------结束状态显示------------------------------------*/

//判断游戏是否结束
void checkover(){
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

//游戏结束显示
void GameOver_Show()
{
	int i, j;
	for (j = 80; j < 260; j++){
		for (i = 160; i < 400; i++){
			XTft_SetPixel(&TftInstance, i, j, gImage_gameover[(j - 80) * 240 + i - 160]);
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

/*-----------------------------主函数------------------------------------*/
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

	Xil_Out32(XPAR_AXI_GPIO_0_BASEADDR+XGPIO_TRI2_OFFSET,0x0);
	Xil_Out32(XPAR_AXI_TIMER_0_BASEADDR+XTC_TCSR_OFFSET,
	    		Xil_In32(XPAR_AXI_TIMER_0_BASEADDR+XTC_TCSR_OFFSET)&~XTC_CSR_ENABLE_TMR_MASK);
	Xil_Out32(XPAR_AXI_TIMER_0_BASEADDR+XTC_TLR_OFFSET,RESET_VALUE);
	Xil_Out32(XPAR_AXI_TIMER_0_BASEADDR+XTC_TCSR_OFFSET,
	    	Xil_In32(XPAR_AXI_TIMER_0_BASEADDR+XTC_TCSR_OFFSET)|XTC_CSR_LOAD_MASK);
	Xil_Out32(XPAR_AXI_TIMER_0_BASEADDR+XTC_TCSR_OFFSET,
	    	(Xil_In32(XPAR_AXI_TIMER_0_BASEADDR+XTC_TCSR_OFFSET)&~XTC_CSR_LOAD_MASK)\
			|XTC_CSR_ENABLE_TMR_MASK|XTC_CSR_AUTO_RELOAD_MASK|XTC_CSR_ENABLE_INT_MASK|XTC_CSR_DOWN_COUNT_MASK);

	TftConfigPtr = XTft_LookupConfig(TFT_DEVICE_ID);
	XTft_CfgInitialize(&TftInstance, TftConfigPtr, TftConfigPtr->BaseAddress); //初始化tft
	XTft_SetFrameBaseAddr(&TftInstance, TFT_FRAME_BASEADDR0);//设置tft的存储基地址
	microblaze_enable_interrupts();//允许微处理器接受中断
	XTft_ClearScreen(&TftInstance);
	clearStack(&stack);
	caculate();
	inimap();
	int i,j=0;
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
/*-----------------------------中断处理------------------------------------*/
void My_ISR()
{
	  int status;
	  status = Xil_In32(XPAR_INTC_0_BASEADDR + XIN_ISR_OFFSET);
	  if ((status & XPAR_AXI_TIMER_0_INTERRUPT_MASK) == XPAR_AXI_TIMER_0_INTERRUPT_MASK)
	    TimerCounterHandler();
	  if ((status & XPAR_AXI_GPIO_0_IP2INTC_IRPT_MASK) == XPAR_AXI_GPIO_0_IP2INTC_IRPT_MASK)
		  Button_Handler();
	  Xil_Out32(XPAR_INTC_0_BASEADDR + XIN_IAR_OFFSET, status);
}

//定时器中断处理
void TimerCounterHandler()
{
	int state=0;//是否能下降
	for (int i = 0;i <4;i++){
		if(map[X + shapes[shapeIndex][i].shape_x][Y + 1 + shapes[shapeIndex][i].shape_y] ==WALL||map[X + shapes[shapeIndex][i].shape_x][Y + 1 + shapes[shapeIndex][i].shape_y] ==fixed_square)
			 {
			 state=0;
			 break;
			 }
		else state=1;
	}

    if(state)
   {
	  for (int i = 0;i <4;i++){
		 map[X + shapes[shapeIndex][i].shape_x][Y + shapes[shapeIndex][i].shape_y] = SPACE;
	  }
	  Y=Y+1;
	  for (int i = 0;i <4;i++){
		     map[X + shapes[shapeIndex][i].shape_x][Y + shapes[shapeIndex][i].shape_y] = move_square;
	  }
   }
	  else
	  {
		for (int i = 0;i <4;i++)
				{
			map[X + shapes[shapeIndex][i].shape_x][Y  + shapes[shapeIndex][i].shape_y] =fixed_square;
			map_color[X + shapes[shapeIndex][i].shape_x][Y  + shapes[shapeIndex][i].shape_y]=shapes[shapeIndex][0].color;
				}
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

//按键中断处理
void Button_Handler(){
	button_status = Xil_In8(XPAR_GPIO_0_BASEADDR + XGPIO_DATA_OFFSET) & 0x1f;  //读取按键的状态值
	if(button_status==Btn_CENTER){
		if (begin == 1)
			begin=0;
		else if(begin == 0)
			begin=1;
		}

	if (begin != 0) {
	    // 如果 begin 不等于 0，说明需要启动定时器
	    Xil_Out32(XPAR_AXI_TIMER_0_BASEADDR + XTC_TCSR_OFFSET,
	              Xil_In32(XPAR_AXI_TIMER_0_BASEADDR + XTC_TCSR_OFFSET) | XTC_CSR_ENABLE_TMR_MASK);
	    makescore_ing();
	}

	if(begin==0){
		// 如果 begin 等于 0，说明需要停止定时器
		Xil_Out32(XPAR_AXI_TIMER_0_BASEADDR+XTC_TCSR_OFFSET,
				Xil_In32(XPAR_AXI_TIMER_0_BASEADDR+XTC_TCSR_OFFSET)&~XTC_CSR_ENABLE_TMR_MASK);
		makescore_stop();//暂停并展示得分
	}

	if (button_status == Btn_UP&&(begin))
		{
		chageShape();
		scan();
		}
	if((button_status==Btn_DOWN)&&(speed_down<speed_max)&&(begin)){//加速下降
			speed_down++;
			Xil_Out32(XPAR_AXI_TIMER_0_BASEADDR+XTC_TLR_OFFSET,(RESET_VALUE+2)/speed_down-2);
		}
		else if((button_status==Btn_DOWN)&&(speed_down==speed_max)&&(begin)){
			speed_down=1;
			Xil_Out32(XPAR_AXI_TIMER_0_BASEADDR+XTC_TLR_OFFSET,(RESET_VALUE+2)/speed_down-2);
		}
	if ((button_status == Btn_LEFT)&&(begin))
		{
		move_left();
		scan();
		}
	if ((button_status == Btn_RIGHT)&&(begin))
		{
		move_right();
		scan();
		}
	Xil_Out32(XPAR_GPIO_0_BASEADDR + XGPIO_ISR_OFFSET, XGPIO_IR_MASK);//写IPISR，清除中断
	Xil_Out32(XPAR_AXI_INTC_0_BASEADDR + XIN_IAR_OFFSET, 0x2);//写INTC_IAR，清除中断状态
}
