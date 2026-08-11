#include "../resources/snake.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <conio.h>

void snake_render(snake_t *snake)
{
    // system("cls"); //注释掉，不要全屏清屏
    COORD pos = {0,0};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);

    for(int y = 0; y < snake->height; y++)
    {
        for(int x = 0; x < snake->width; x++)
        {
            int is_body = 0;
            int is_head = 0;
            int is_food = 0;

            if(x == snake->food_x && y == snake->food_y)
            {
                is_food = 1;
            }

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

            if(is_head)
                printf("@");
            else if(is_body)
                printf("O");
            else if(is_food)
                printf("*");
            else
                printf(".");
        }
        printf("\n");
    }
    if(snake->game_over)
    {
        printf("GAME OVER!\n");
    }
}

int main(void)
{
    srand((unsigned)time(NULL));

    snake_t *game = snake_create(40,30);
    if(game == NULL)
    {
        fprintf(stderr, "snake_create failed\n");
        return 1;
    }

    // 控制蛇移动间隔：300ms走一步，可以调大更慢，调小更快
    const DWORD move_interval = 300;
    DWORD last_move_time = GetTickCount();

    while(!game->game_over)
    {
        // 不停检测按键，不需要等待Sleep，按键立刻响应
        if(_kbhit())
        {
            char ch = _getch();
            switch(ch)
            {
                case 'w': snake_change_dir(game, SNAKE_DIR_UP); break;
                case 's': snake_change_dir(game, SNAKE_DIR_DOWN); break;
                case 'a': snake_change_dir(game, SNAKE_DIR_LEFT); break;
                case 'd': snake_change_dir(game, SNAKE_DIR_RIGHT); break;
                case 'q': game->game_over = 1; break;
            }
        }

        DWORD now = GetTickCount();
        // 时间到了才移动蛇，不是每次循环都移动
        if(now - last_move_time >= move_interval)
        {
            snake_move(game);
            last_move_time = now;
            snake_render(game);
        }
        // 小sleep降低CPU占用，10ms，几乎不影响按键响应
        Sleep(10);
    }
    snake_destroy(game);
    return 0;
}