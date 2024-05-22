#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<unistd.h>
#include <time.h>
 
#define TTY_PATH  "/dev/tty"                      //系统指令
#define STTY_US  "stty raw -echo -F "      //系统指令
#define STTY_DEF  "stty -raw echo -F "    //系统指令
 
#define WIDE 18
#define HEIGHT 20
#define INFO_AREA 6
#define SPEED 8
#define Y_NEXT 4                             // x_next, y_next下一个图形在方框的位置
#define X_NEXT WIDE - INFO_AREA / 2 - 1
 
int score = 0;                                       //分数
int map[HEIGHT][WIDE] = { 0 };  //界面

int x, y;                                                  // x, y 正在下降图形的位置
int moveDown_count = 0;
int shapeIndex, shapeIndex_next;  //shapeIndex：下移图形 shapeIndex_next：下一个图形
 
struct Point {						             //中心点的偏移量，中心点以此延伸获得各形状点的坐标
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
 
void setFrame();              //构建边宽
void showMap();             //展示界面
void showPoint(int x, int y,int bright,int color);
void showShape();         //输出图形
void creatNewShape();//创建新的图形
int moveDown();             //图形下降，判断是否还可以移动
void addToMap();           //把确定位置的保存在map中
int overCheak();              //确定游戏是否结束
int get_char();                  //获取输入
void control(char str);  //控制移动
void chageShape();       //改变形状 
void moveLeft();             //左移
void moveRight();          //右移
void clearLines();           //清除已满行

int main()
{
    system(STTY_US TTY_PATH);  //输入阻塞
    setFrame();                                      //初始化map
    showMap();                                     //展示界面
    srand(time(0));
    shapeIndex = rand() % 19;
    creatNewShape();                        //创建下一个图形
    
    while(1) {
        system("clear");                        //清除界面显示内容
        showShape();                             //展示方块
        showMap();                                 //展示界面
        if (moveDown()) {                     //判断移动
            addToMap();                           //不可移动后，写入到map
            if (overCheak()) {                   //检查是否游戏结束
                break;
            }
            clearLines();                            //判断一行是否满，并清理
            shapeIndex = shapeIndex_next;
            creatNewShape();                //创建下一个图形
        }
        char str = get_char();              //得到输入指令
        if (str == 3) {                                 //是否为ctrl+C
            break;
        }
        control(str);                                 //控制移动
        usleep(1000*50);                       //阻碍运行速度
    }
    system(STTY_DEF TTY_PATH); //输入阻塞结束
    printf("\033[21;0H");
}
 
/**
 * @brief Set the Frame object 
 */
void setFrame()
{
    int i;
    for(i = 0; i < WIDE; i++) {              //界面两条横线
        map[0][i] = 1;
        map[HEIGHT - 1][i] = 1;
    }
    for(i = 0; i < HEIGHT; i++) {        //界面三条竖线
        map[i][0] = 1;
        map[i][WIDE - INFO_AREA - 1] = 1;
        map[i] [WIDE - 1] = 1;
    }
}
 
/**
 * @brief 展示界面 
 */
void showMap()//已经加上去
{
    int i, j;
	for(i = 0; i < HEIGHT; i++) {
		for(j = 0;j < WIDE; j++) {
			if ( map[i][j] == 1 || map[i][j] == 2) {
				showPoint(j, i, 1, 32 + map[i][j]);   //行标i，列表j与坐标系横纵坐标的值是相反的
			}
		}
	} 
	printf("\033[2;27H");          //   \033[X;XH  光标移动格式
	printf("\033[31mNext:"); 
 
	printf("\033[11;27H"); 
	printf("\033[32mScore:");   
 
	printf("\033[12;26H"); 
	printf(" \033[32m%3d", score);   
	fflush(stdout);  //清理缓存，使画面动作流畅
}
 
/**
 * @brief 输出一个方块
 * 
 * @param x1 光标横轴位置
 * @param y1 光标纵轴位置
 * @param bright 颜色亮度
 * @param color   颜色种类
 */
void showPoint(int x1, int y1, int bright, int color)//已经加上去
{
    printf("\033[%d;%dH", y1 + 1, x1 * 2 + 1);                   //光标位置
    printf("\033[%d;%dm■ \033[0m", bright, color);  //颜色深浅，颜色种类
}
 
/**
 * @brief 创建下一个图形和重置光标位置
 */
void creatNewShape()//已经加上去
{ 
    y = 2;
    x = (WIDE - INFO_AREA) / 2;
    shapeIndex_next = rand() % 19;
}
 
 
/**
 * @brief  显示正在下降的图形和显示下一个图形
 */
void showShape()//已经加上去
{
    int i;
    for(i = 0; i < 4; i++) {     //显示正在下降的图形
        showPoint(x + shapes[shapeIndex][i].shape_x, y + shapes[shapeIndex][i].shape_y, 1, 31);
    }
    for(i = 0; i < 4; i++) {     //显示下一个图形
        showPoint(X_NEXT + shapes[shapeIndex_next][i].shape_x, Y_NEXT + shapes[shapeIndex_next][i].shape_y, 1, 31);
    }
}
 
/**
 * @brief 图形下降和控制移速
 * 
 * @return int 1为已经处地，0还可移动
 */
int moveDown()
{
    int i;
    if (moveDown_count < SPEED) {   //控制移动速度
		moveDown_count++;                    //移动准确性
		return 0;                                               //也就是两次进入函数图形下移一格
	}
	moveDown_count = 0;
	for (i = 0; i < 4; i++) {
		int dx = x + shapes[shapeIndex][i].shape_x;
		int dy = y + shapes[shapeIndex][i].shape_y + 1;//原来位置的map[x][y]已经是１
		
		if (map[dy][dx] == 1 || map[dy][dx] == 2) {
			return 1;                     //不可移动
		}
	}
    y++;
    return 0;
}
 
/**
 * @brief  把不能移动的方块保存到map中
 */
void addToMap()
{
    int i;
    for(i = 0; i < 4; i++) {
        map[y + shapes[shapeIndex][i].shape_y][x + shapes[shapeIndex][i].shape_x] = 2;
    }
}
 
/**
 * @brief 检测游戏是否结束
 * 
 * @return int 0为结束，1还可继续
 */
int overCheak()
{
    int i;
    for (i = 0; i < WIDE; i++) {
        if (map[1][i] == 2) {
            return 1;
        }
    }
    return 0;
}
 
/**
 * @brief Get the char object
 * 
 * @return int 返回输入的字符对应的ASCII值
 */
int get_char()	
{
    fd_set rfds;
    struct timeval tv;
    int ch = 0;
    FD_ZERO(&rfds);
   FD_SET(0, &rfds);
    tv.tv_sec = 0;
    tv.tv_usec = 10; 
    if (select(1, &rfds, NULL, NULL, &tv) > 0) {
        ch = getchar(); 
    }
    return ch;
}
 
/**
 * @brief 判断输入，得出下一步移动
 * 
 * @param str 输入的字符
 */
void control(char str)//已经加上去
{
    switch (str) {
    case 'w':                          //变形
        chageShape();
        break;
    case 'a':                           //左移
        moveLeft();
        break;
    case 's':                            //下移
        moveDown_count = moveDown_count + SPEED;
        break;
    case 'd':                            //右移
        moveRight();
        break;
    }
}
 
/**
 * @brief  改变形状
 * 原理：通过改变shapeIndex的值
 */
void chageShape()//已经加上去
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
	int i; 
	//判断假设的图形是否发生碰撞
	for(i = 0;i < 4;i++) {
		int cx = x+shapes[ts][i].shape_x;
		int cy = y+shapes[ts][i].shape_y;
		if (map[cy][cx]==1 || map [cy][cx]==2) {
			return;
		}
	}
	shapeIndex = ts; //完成变形
}
 
/**
 * @brief  左移
 */
void moveLeft()//已经加上去
{
    int i;
    for (i = 0; i < 4; i++) {
        if(map[y + shapes[shapeIndex][i].shape_y][x + shapes[shapeIndex][i].shape_x - 1] == 1
        || map[y + shapes[shapeIndex][i].shape_y][x + shapes[shapeIndex][i].shape_x - 1] == 2) {
            return;
        }
    }
    x--;
}
 
/**
 * @brief 右移
 */
void moveRight()//已经加上去
{
    int i;
    for(i = 0; i < 4; i++) {
        if (map[y + shapes[shapeIndex][i].shape_y][x + shapes[shapeIndex][i].shape_x + 1] == 1
        || map[y + shapes[shapeIndex][i].shape_y][x + shapes[shapeIndex][i].shape_x + 1] == 2) {
            return;
        }
    }
    x++;
}
 
/**
 * @brief  判断一行是否满，并清理，拷贝上面内容
 */
void clearLines() 
{
    int i, j, a;
    for (i = HEIGHT - 2; i > 0; i--) {
        a = 1;
        for(j = 1; j < WIDE - INFO_AREA - 1;  j++) {  //确定是否一行排满
            if (map[i][j] != 2) {
                a = 0;
                break;
            } 
        }
    
        if (a == 1) {
            score++;
            int n;
            for(n = i; n > 1 ; n--) {   //把上面的拷贝下来
                for(j = 0; j < WIDE - INFO_AREA - 1;  j++) {
                    map[n][j] = map[n- 1][j];
                }
            }
            i++;              //防止有多行一起排满
        }
    }
    fflush(stdout); //清理缓存，使画面动作流畅
}
 