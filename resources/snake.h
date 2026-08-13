#ifndef __SNAKE_H__
#define __SNAKE_H__

#include "../base/linked_list.h"

typedef enum 
{
    SNAKE_DIR_UP = 1,
    SNAKE_DIR_DOWN = 2,
    SNAKE_DIR_LEFT = 3,
    SNAKE_DIR_RIGHT = 4
}snake_dir_t;

typedef struct 
{
    int x;
    int y;
} snake_body_t;


typedef struct
{
    list_t *body;
    snake_dir_t dir;
    int food_x;
    int food_y;
    int game_over;
    int width;
    int height;
    uint32_t snake_len;
    uint32_t grade;
}snake_t;

snake_t *snake_create(int w, int h);
void snake_destroy(snake_t *snake);
void snake_change_dir(snake_t *snake, snake_dir_t new_dir);
void snake_move(snake_t *snake);
void snake_gen_food(snake_t *snake);
uint32_t snake_get_len(snake_t *snake);
uint32_t snake_get_grade(uint32_t snake_len);

#endif