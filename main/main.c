#include "../resources/snake.h"
#include "../base/linked_list.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>   // Windows系统API：控制台光标、GetTickCount、Sleep、GetAsyncKeyState
#include <conio.h>     // 控制台非阻塞按键：_kbhit() _getch()


// =====================全局变量=====================
DWORD move_interval = 300;          // 蛇每次移动的时间间隔，单位毫秒，会动态修改
const DWORD NORMAL_INTERVAL = 300;  // 正常速度：300ms走一步
const DWORD BOOST_INTERVAL = 100;   // 按住空格加速：100ms走一步
const DWORD MIN_INTERVAL = 50;      // 速度下限，蛇最小移动间隔

/**
 * @brief 渲染整个游戏画面，把蛇、食物、边界打印到控制台
 * @param snake 游戏实例指针，保存地图宽高、蛇链表、食物坐标、游戏结束标记
 * 实现思路：不频繁清屏！直接把光标挪到控制台左上角，整幅画面重新覆盖输出，消除闪烁
 */
void snake_render(snake_t *snake)
{
    // ==========隐藏控制台光标，消除闪烁的小技巧==========
    CONSOLE_CURSOR_INFO cursor_info;
    // 获取控制台当前光标属性
    GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);
    cursor_info.bVisible = FALSE;   // 设置光标不可见，不然闪烁会有一闪一闪下划线
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);

    // 把控制台光标移动到坐标(0,0)，也就是窗口左上角
    COORD pos = {0, 0};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);

    char line_buf[64];  // 一行地图的字符缓冲区，减少频繁printf调用，提升速度

    // 双重循环遍历整张地图 y行 x列
    for(int y = 0; y < snake->height; y++)
    {
        int idx = 0;   // line_buf数组下标，记录当前写到哪个位置
        for(int x = 0; x < snake->width; x++)
        {
            int is_body = 0;   // 当前坐标是不是蛇身体
            int is_head = 0;   // 当前坐标是不是蛇头
            int is_food = 0;   // 当前坐标是不是食物

            // 判断是否是食物
            if(x == snake->food_x && y == snake->food_y)
            {
                is_food = 1;
            }

            // 遍历蛇身体链表，判断当前(x,y)是不是蛇头/蛇身体
            list_node_t *cur = snake->body->head;   // 拿到链表头节点（蛇头）
            if(cur != NULL)    // 链表不为空，蛇存在
            {
                // 头节点存蛇头坐标
                snake_body_t *h = (snake_body_t*)cur->data;
                if(h->x == x && h->y == y)
                    is_head = 1;
                cur = cur->next;  // 跳到下一个节点，开始遍历蛇身体，跳过蛇头
                while(cur != NULL)
                {
                    snake_body_t *b = (snake_body_t*)cur->data;
                    if(b->x == x && b->y == y)
                    {
                        is_body = 1;
                    }
                    cur = cur->next; // 继续下一节蛇身
                }
            }

            // 根据标记填充字符缓冲区
            if(is_head)
                line_buf[idx++] = '@';     // 蛇头用@符号
            else if(is_body)
                line_buf[idx++] = 'O';     // 蛇身体O
            else if(is_food)
                line_buf[idx++] = '*';     // 食物*
            else
                line_buf[idx++] = '.';     // 空地.
        }
        line_buf[idx++] = '\n';   // 这一行结束，换行
        line_buf[idx] = '\0';     // 字符串结束符，printf识别结束位置
        printf("%s", line_buf);   // 一次性打印整行
    }

    printf("Speed: %d ms | grade: %d | O: quit\n", move_interval, snake->grade);
    if(snake->game_over)
    {
        printf("GAME OVER!\n");
    }
}


int main(void)
{
    srand((unsigned)time(NULL));   // 设置随机数种子，食物随机生成要用到，time拿系统时间做种子

    // 创建游戏实例，地图宽50，高30；内部会初始化蛇链表、生成第一个食物
    snake_t *game = snake_create(50,30);
    if(game == NULL)
    {
        fprintf(stderr, "snake_create failed\n");
        return 1;
    }

    DWORD last_move_time = GetTickCount(); // 记录上一次蛇移动的时间戳，单位ms

    // =================游戏主循环=================
    while(!game->game_over)
    {
        // ----------第一部分：处理WASD Q按键（按键按下一瞬间触发）----------
        // _kbhit()：检测键盘有没有按键按下，有返回非0，没有返回0；非阻塞，不会卡住程序
        if(_kbhit())
        {
            char ch = _getch();  // 读取按下的按键字符，不需要回车，控制台直接捕获键盘
            switch(ch)
            {
                case 'W': snake_change_dir(game, SNAKE_DIR_UP); break;    // W向上
                case 'S': snake_change_dir(game, SNAKE_DIR_DOWN); break;  // S向下
                case 'A': snake_change_dir(game, SNAKE_DIR_LEFT); break;  // A向左
                case 'D': snake_change_dir(game, SNAKE_DIR_RIGHT); break; // D向右
                case 'O': game->game_over = 1; break;                     // O设置游戏结束，退出循环
            }
        }

        // ----------第二部分：实时检测空格键是否正在按住----------
        // 仅记录按键当前状态到标志位，真实的间隔由分数阈值逻辑统一计算后再应用，避免被覆盖
        int space_pressed = (GetAsyncKeyState(VK_SPACE) & 0x8000) ? 1 : 0;

        // ----------第三部分：时间判断，控制蛇多久走一步【核心帧逻辑】----------
        DWORD now = GetTickCount(); // 获取系统开机到现在经过的毫秒数，WindowsAPI
        // 当前时间 - 上次移动时间 >= 设定间隔，才执行一次移动+渲染
        if(now - last_move_time >= move_interval)
        {
            snake_move(game);        // 蛇移动一步，内部逻辑：头前进、判断吃食物、撞墙撞自己判定
            last_move_time = now;    // 更新移动时间戳
            snake_render(game);      // 移动完成后刷新画面
        }

        DWORD base_interval = NORMAL_INTERVAL;
        if (game->grade >= 80)
        {
            base_interval = 100;  // 分数80以上，基础速度100ms
        }
        else if (game->grade >= 65)
        {
            base_interval = 150;  // 分数65以上，基础速度150ms
        }
        else if (game->grade >= 50)
        {
            base_interval = 200;  // 分数50以上，基础速度200ms
        }
        else if (game->grade >= 30)
        {
            base_interval = 250;  // 分数30以上，基础速度250ms
        }

        // 最终应用移动间隔：若空格正在按住则使用加速间隔，否则使用基础间隔
        if (space_pressed)
            move_interval = BOOST_INTERVAL;
        else
            move_interval = base_interval;

        //最小速度保护，不能比MIN_INTERVAL更小
        if(move_interval < MIN_INTERVAL)
        {
            move_interval = MIN_INTERVAL;
        }

        Sleep(10); // 休眠10ms，降低CPU占用，不要写很大的值！
        // 这里Sleep不是控制蛇移动！只是让循环不要疯狂空转，10ms轮询一次键盘。
        // 蛇移动完全靠GetTickCount时间戳判断，这叫【非阻塞时间驱动】
    }

    //游戏结束等待按键，防止控制台窗口一闪而过看不到GAME OVER
    printf("\nGame Over, press any key to exit...");
    (void)_getch();

    // 游戏退出，释放所有动态内存，蛇链表、snake_t结构体全部销毁
    snake_destroy(game);
    return 0;
}