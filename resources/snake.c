#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "snake.h"

static void snake_body_free(void *data)
{
    if(data != NULL)
    {
        free(data);
    }
}

snake_t *snake_create(int w, int h)
{
    snake_t *snake = (snake_t*)malloc(sizeof(snake_t));
    if(snake == NULL)           return NULL;

    snake->width = w;
    snake->height = h;
    snake->game_over = 0;
    snake->dir = SNAKE_DIR_UP;

    snake->body = list_create(snake_body_free);
    if(snake->body == NULL)
    {
        free(snake);
        return NULL;
    }

    snake_body_t *b1 = (snake_body_t*)malloc(sizeof(snake_body_t));
    b1->x = w / 2;
    b1->y = h / 2 + 2;  
    list_push_front(snake->body, b1);

    snake_body_t *b2 = (snake_body_t*)malloc(sizeof(snake_body_t));
    b2->x = w / 2;
    b2->y = h / 2 + 1;
    list_push_front(snake->body, b2);

    snake_body_t *b3 = (snake_body_t*)malloc(sizeof(snake_body_t));
    b3->x = w / 2;
    b3->y = h / 2;
    list_push_front(snake->body, b3);

    snake_gen_food(snake);
    
    return snake;
}

void snake_destroy(snake_t *snake)
{
    if(snake != NULL)
    {
        list_destroy(snake->body);
        free(snake);
    }
}

void snake_change_dir(snake_t *snake, snake_dir_t new_dir)
{
    if(snake == NULL)   return;

    if((snake->dir == SNAKE_DIR_UP && new_dir == SNAKE_DIR_DOWN) ||
       (snake->dir == SNAKE_DIR_DOWN && new_dir == SNAKE_DIR_UP) ||
       (snake->dir == SNAKE_DIR_LEFT && new_dir == SNAKE_DIR_RIGHT) ||
       (snake->dir == SNAKE_DIR_RIGHT && new_dir == SNAKE_DIR_LEFT))
    {
        return;
    }

    snake->dir = new_dir;
}

void snake_move(snake_t *snake)
{
    if(snake == NULL || snake->game_over)   return;

    list_node_t *head_node = snake->body->head;
    snake_body_t *head = (snake_body_t*)head_node->data;

    snake_body_t *new_head = (snake_body_t*)malloc(sizeof(snake_body_t));
    new_head->x = head->x;
    new_head->y = head->y;

    switch(snake->dir)
    {
        case SNAKE_DIR_UP:    new_head->y -= 1;     break;
        case SNAKE_DIR_DOWN:  new_head->y += 1;     break;
        case SNAKE_DIR_LEFT:  new_head->x -= 1;     break;
        case SNAKE_DIR_RIGHT: new_head->x += 1;     break;
    }

    if(new_head->x < 0 || new_head->x >= snake->width || new_head->y < 0 || new_head->y >= snake->height)
    {
        snake->game_over = 1;
        free(new_head);
        return;
    }

    list_node_t *cur = snake->body->head;
    while(cur != NULL)
    {
        snake_body_t *body_part = (snake_body_t*)cur->data;
        if(body_part->x == new_head->x && body_part->y == new_head->y)
        {
            snake->game_over = 1;
            free(new_head);
            return;
        }
        cur = cur->next;
    }
    
    list_push_front(snake->body, new_head);

    if(new_head->x == snake->food_x && new_head->y == snake->food_y)
    {
        snake_gen_food(snake);
    }
    else
    {
        list_pop_back(snake->body);
    }

}

void snake_gen_food(snake_t *snake)
{
    if(snake == NULL)   return;
    int ok;
    do
    {
        ok = 1;
        snake->food_x = rand() % snake->width;
        snake->food_y = rand() % snake->height;

        list_node_t *cur = snake->body->head;
        while(cur != NULL)
        {
            snake_body_t *body_part = (snake_body_t*)cur->data;
            if(body_part->x == snake->food_x && body_part->y == snake->food_y)
            {
                ok = 0;
                break;
            }
            cur = cur->next;
        }
    } while(!ok);
    
}