#include "../resources/snake.h"
#include "../base/linked_list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include <conio.h>

// =====================全局变量=====================
DWORD move_interval = 300;          // 蛇移动时间间隔(ms)
const DWORD NORMAL_INTERVAL = 300;  // 正常移动间隔
const DWORD BOOST_INTERVAL = 100;   // 空格加速移动间隔
const DWORD MIN_INTERVAL = 50;      // 最小移动间隔，防止速度过快

int need_redraw_menu = 1;           // 菜单重绘标记，1=需要刷新菜单界面
static int g_original_caps_state;   // 保存程序启动前CapsLock大写锁定原始状态

/**
 * @brief 模拟按下/松开CapsLock按键，切换大写锁定状态
 */
static void ToggleCapsLock(void)
{
    keybd_event(VK_CAPITAL, 0x45, 0, 0);                // 按下CapsLock
    keybd_event(VK_CAPITAL, 0x45, KEYEVENTF_KEYUP, 0);  // 松开CapsLock
}

/**
 * @brief 程序退出回调函数，程序正常退出时自动执行
 * 恢复用户运行程序之前的CapsLock大写锁定状态
 */
static void RestoreCapsLockOnExit(void)
{
    SHORT current = GetKeyState(VK_CAPITAL);
    int is_caps_on = (current & 0x0001) ? 1 : 0;
    // 如果当前状态和启动原始状态不一致，则切换回来
    if(is_caps_on != g_original_caps_state)
    {
        ToggleCapsLock();
    }
}

/**
 * @brief 渲染游戏地图画面
 * @param snake 蛇游戏实例指针
 * 功能：将蛇身体、蛇头、食物、空地输出到控制台，光标固定在左上角，实现无闪烁刷新
 */
void snake_render(snake_t *snake)
{
    CONSOLE_CURSOR_INFO cursor_info;
    GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);
    cursor_info.bVisible = FALSE;               // 隐藏控制台光标，消除光标闪烁
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);

    COORD pos = {0, 0};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos); // 光标定位到窗口左上角

    char line_buf[64];
    // 逐行遍历地图Y轴
    for(int y = 0; y < snake->height; y++)
    {
        int idx = 0;
        // 遍历该行X轴每一个格子
        for(int x = 0; x < snake->width; x++)
        {
            int is_body = 0;
            int is_head = 0;
            int is_food = 0;

            // 判断当前坐标是否为食物
            if(x == snake->food_x && y == snake->food_y)
            {
                is_food = 1;
            }

            // 遍历蛇链表，判断当前坐标是否是蛇头/蛇身体
            list_node_t *cur = snake->body->head;
            if(cur != NULL)
            {
                snake_body_t *h = (snake_body_t*)cur->data;
                if(h->x == x && h->y == y)
                    is_head = 1;
                cur = cur->next;
                while(cur != NULL)
                {
                    snake_body_t *b = (snake_body_t*)cur->data;
                    if(b->x == x && b->y == y)
                    {
                        is_body = 1;
                    }
                    cur = cur->next;
                }
            }

            // 填充对应字符：@蛇头 O蛇身体 *食物 .空地
            if(is_head)
                line_buf[idx++] = '@';
            else if(is_body)
                line_buf[idx++] = 'O';
            else if(is_food)
                line_buf[idx++] = '*';
            else
                line_buf[idx++] = '.';
        }
        line_buf[idx++] = '\n';
        line_buf[idx] = '\0';
        printf("%s", line_buf);
    }

    // 底部状态栏输出：当前速度、得分、操作提示
    printf("Speed: %d ms | Grade: %d | O: quit current game\n", move_interval, snake->grade);
    if(snake->game_over)
    {
        printf("GAME OVER!\n");
    }
}

/**
 * @brief 打印主菜单界面
 * 使用system("cls")清屏，输出菜单选项
 */
void show_menu(void)
{
    system("cls");
    printf("===============");
    printf(" SNAKE GAME ");
    printf("===============\n");
    printf("[N]   Start New Game\n");
    printf("[O]   Exit Program\n");
    printf("[H]   Show Help\n");
    printf("==================================\n");
}

/**
 * @brief 游戏帮助说明页面
 * 输出按键说明、游戏规则，等待任意按键返回菜单
 */
void show_help(void)
{
    system("cls");
    printf("================");
    printf(" GAME HELP ");
    printf("================\n");

    printf("CONTROLS\n");
    printf("  W      Move Up\n");
    printf("  S      Move Down\n");
    printf("  A      Move Left\n");
    printf("  D      Move Right\n");
    printf(" SPACE   Hold to speed boost\n");
    printf("  O      Quit current game\n");
    printf("\n");

    printf("RULES\n");
    printf("1. Eat food '*' to gain grade and grow longer\n");
    printf("2. Higher grade brings faster base movement speed\n");
    printf("3. Hitting wall or your own body causes game over\n");
    printf("4. Speed boost only works while holding SPACE\n");
    printf("\n");

    printf("MENU KEYS\n");
    printf(" N  Start new game\n");
    printf(" O  Exit whole program\n");
    printf("\n");

    printf("================================\n");
    printf(" Press any key back to menu...\n");
}

int main(void)
{
    // 读取程序启动瞬间CapsLock原始状态，用于退出后还原
    g_original_caps_state = (GetKeyState(VK_CAPITAL) & 0x0001) ? 1 : 0;
    // 注册atexit回调，程序正常return退出时自动调用恢复大写状态
    atexit(RestoreCapsLockOnExit);
    // 如果启动前大写未开启，则自动打开CapsLock
    if(g_original_caps_state == 0)
    {
        ToggleCapsLock();
    }

    // 设置控制台窗口大小，适配50×30地图，预留状态栏边距
    system("mode con cols=62 lines=36");
    system("title Snake Game");

    srand((unsigned)time(NULL));    // 设置随机数种子，用于食物随机生成

    snake_t *game = NULL;           // 游戏实例指针，NULL代表处于菜单状态

    // 主循环：菜单 ↔ 游戏状态切换
    while(1)
    {
        // 仅当标记置1，才绘制一次菜单，避免循环反复cls造成屏幕闪烁
        if(need_redraw_menu)
        {
            show_menu();
            need_redraw_menu = 0;
        }

        // _kbhit检测是否有键盘按键按下
        if(_kbhit())
        {
            char ch = _getch();     // _getch读取按键，不需要回车
            switch(ch)
            {
                case 'N':
                case 'n':
                    // 如果已有游戏实例，先销毁释放内存
                    if(game != NULL)
                    {
                        snake_destroy(game);
                        game = NULL;
                    }
                    // 创建50宽30高的游戏地图
                    game = snake_create(50,30);
                    if(game != NULL)
                    {
                        game->game_over = 0;
                        move_interval = NORMAL_INTERVAL; // 新建游戏重置移动速度
                    }
                    break;
                case 'O':
                case 'o':
                    // 退出程序，释放游戏内存
                    if(game != NULL)
                    {
                        snake_destroy(game);
                    }
                    return 0;
                case 'H':
                case 'h':
                    show_help();
                    (void)_getch();             // 阻塞等待用户按下任意键
                    need_redraw_menu = 1;       // 标记需要重新绘制主菜单
                    break;
            }
        }

        // game为NULL代表菜单状态，休眠降低CPU占用，继续循环
        if(game == NULL)
        {
            Sleep(10);
            continue;
        }

        DWORD last_move_time = GetTickCount();  // 记录上一次蛇移动的时间戳

        // 游戏内层循环，game_over为0持续运行游戏
        while(!game->game_over)
        {
            // 游戏内按键检测：W/S/A/D改变方向，O退出本局
            if(_kbhit())
            {
                char ch = _getch();
                switch(ch)
                {
                    case 'W': snake_change_dir(game, SNAKE_DIR_UP); break;
                    case 'S': snake_change_dir(game, SNAKE_DIR_DOWN); break;
                    case 'A': snake_change_dir(game, SNAKE_DIR_LEFT); break;
                    case 'D': snake_change_dir(game, SNAKE_DIR_RIGHT); break;
                    case 'O': game->game_over = 1; break;
                }
            }

            // 判断空格键是否按下，用来加速
            int space_pressed = (GetAsyncKeyState(VK_SPACE) & 0x8000) ? 1 : 0;
            DWORD now = GetTickCount();

            // 判断时间间隔，到达时间执行蛇移动+画面刷新
            if(now - last_move_time >= move_interval)
            {
                snake_move(game);
                last_move_time = now;
                snake_render(game);
            }

            // 根据得分动态调整基础移动速度，分数越高速度越快
            DWORD base_interval = NORMAL_INTERVAL;
            if (game->grade >= 80)
            {
                base_interval = 100;
            }
            else if (game->grade >= 65)
            {
                base_interval = 150;
            }
            else if (game->grade >= 50)
            {
                base_interval = 200;
            }
            else if (game->grade >= 30)
            {
                base_interval = 250;
            }

            // 空格按下使用加速间隔，否则使用得分对应的基础间隔
            if (space_pressed)
                move_interval = BOOST_INTERVAL;
            else
                move_interval = base_interval;

            // 速度下限保护，不允许小于最小间隔
            if(move_interval < MIN_INTERVAL)
            {
                move_interval = MIN_INTERVAL;
            }

            Sleep(10);
        }

        // =================游戏结束处理=================
        system("cls");
        printf("\nyour final grade: %d\n", game->grade);      // 输出本局最终得分
        printf("\nGame Over, press any key back to menu...\n");
        (void)_getch();                                     // 阻塞等待按键，用户确认后返回菜单

        snake_destroy(game);    // 销毁蛇游戏实例，释放链表内存
        game = NULL;

        need_redraw_menu = 1;   // 标记需要重绘菜单，回到外层循环刷新主菜单
    }
    return 0;
}