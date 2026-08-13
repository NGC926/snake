#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "snake.h"

/**
 * @brief 链表销毁回调函数，用来释放蛇身体节点的数据内存
 * @param data 链表节点内保存的snake_body_t*指针
 * 这个函数会被list_destroy内部自动调用，每一个节点的data都会传入这里
 */
static void snake_body_free(void *data)
{
    if(data != NULL)
    {
        free(data);   // 释放每一节蛇身体堆内存
    }
}

/**
 * @brief 创建贪吃蛇游戏实例，初始化蛇身体、生成初始食物
 * @param w 地图宽度（x方向）
 * @param h 地图高度（y方向）
 * @return 成功返回snake_t指针；内存分配失败返回NULL
 */
snake_t *snake_create(int w, int h)
{
    // 分配游戏总结构体内存
    snake_t *snake = (snake_t*)malloc(sizeof(snake_t));
    if(snake == NULL)
        return NULL;

    // 初始化地图宽高，游戏未结束，初始移动方向向上
    snake->width = w;
    snake->height = h;
    snake->game_over = 0;
    snake->dir = SNAKE_DIR_UP;

    //初始化蛇长度与分数，初始蛇3节身体
    snake->snake_len = 3;
    snake->grade = 0;

    // 创建双向链表，传入销毁回调：链表销毁时自动调用snake_body_free释放每一节蛇身体
    snake->body = list_create(snake_body_free);
    if(snake->body == NULL)
    {
        // 链表创建失败，要手动释放已经分配好的snake结构体，防止内存泄漏
        free(snake);
        return NULL;
    }

    // 生成3节蛇身体，从尾部往头部push_front
    // 注意list_push_front是插入链表头部；后push的节点会变成新蛇头
    // b1：蛇尾巴
    snake_body_t *b1 = (snake_body_t*)malloc(sizeof(snake_body_t));
    b1->x = w / 2;
    b1->y = h / 2 + 2;
    list_push_front(snake->body, b1);

    // b2：中间一节
    snake_body_t *b2 = (snake_body_t*)malloc(sizeof(snake_body_t));
    b2->x = w / 2;
    b2->y = h / 2 + 1;
    list_push_front(snake->body, b2);

    // b3：蛇头，最后push_front，处于链表head位置
    snake_body_t *b3 = (snake_body_t*)malloc(sizeof(snake_body_t));
    b3->x = w / 2;
    b3->y = h / 2;
    list_push_front(snake->body, b3);

    // 创建完毕，生成第一份食物
    snake_gen_food(snake);

    return snake;
}

/**
 * @brief 销毁整个游戏对象，释放全部堆内存
 * @param snake 游戏实例指针
 */
void snake_destroy(snake_t *snake)
{
    if(snake != NULL)
    {
        // list_destroy会遍历链表，调用注册好的snake_body_free释放每一节蛇身体，再释放链表本身
        list_destroy(snake->body);
        // 释放游戏主结构体
        free(snake);
    }
}

/**
 * @brief 修改蛇移动方向，禁止直接180度掉头（蛇向上不能直接向下，向左不能直接向右）
 * @param snake 游戏实例
 * @param new_dir 用户想要切换的新方向
 */
void snake_change_dir(snake_t *snake, snake_dir_t new_dir)
{
    if(snake == NULL)
        return;

    // 判断：如果新方向是当前方向的反方向，直接return，拒绝修改方向
    if((snake->dir == SNAKE_DIR_UP && new_dir == SNAKE_DIR_DOWN) ||
       (snake->dir == SNAKE_DIR_DOWN && new_dir == SNAKE_DIR_UP) ||
       (snake->dir == SNAKE_DIR_LEFT && new_dir == SNAKE_DIR_RIGHT) ||
       (snake->dir == SNAKE_DIR_RIGHT && new_dir == SNAKE_DIR_LEFT))
    {
        return;
    }

    // 合法方向，更新蛇的移动方向
    snake->dir = new_dir;
}

/**
 * @brief 获取蛇当前身体节数
 * @param snake 游戏实例
 * @return 蛇身体节点数量
 */
uint32_t snake_get_len(snake_t *snake)
{
    if(snake == NULL || snake->body == NULL)
        return 0;
    return list_size(snake->body);
}

/**
 * @brief 根据蛇长度计算分数
 * @param snake_len 蛇身体节数
 * @return 计算得到的分数
 */
uint32_t snake_get_grade(uint32_t snake_len)
{
    if(snake_len < 3)
        return 0;
    return (snake_len - 3) * 2;   // 蛇初始长度3节，吃一个食物长一节，分数=(蛇长度-3) * 2
}

/**
 * @brief 蛇执行一次移动逻辑
 * 1. 根据当前方向算出新蛇头坐标
 * 2. 撞墙检测，撞墙置game_over
 * 3. 撞到自己身体检测，撞到置game_over
 * 4. 新蛇头插入链表头部
 * 5. 判断是否吃到食物：吃到则不删尾巴，自动生成新食物；没吃到弹出尾部节点，蛇整体平移
 * @param snake 游戏实例
 */
void snake_move(snake_t *snake)
{
    // 空指针或者游戏已经结束，直接退出，不执行移动
    if(snake == NULL || snake->game_over)
        return;

    // 获取当前旧蛇头节点
    list_node_t *head_node = snake->body->head;
    snake_body_t *head = (snake_body_t*)head_node->data;

    // 分配内存，构造【新蛇头】，初始复制旧蛇头坐标
    snake_body_t *new_head = (snake_body_t*)malloc(sizeof(snake_body_t));
    new_head->x = head->x;
    new_head->y = head->y;

    // 根据移动方向修改新蛇头坐标
    switch(snake->dir)
    {
        case SNAKE_DIR_UP:    new_head->y -= 1;     break;
        case SNAKE_DIR_DOWN:  new_head->y += 1;     break;
        case SNAKE_DIR_LEFT:  new_head->x -= 1;     break;
        case SNAKE_DIR_RIGHT: new_head->x += 1;     break;
    }

    // =========撞墙判定=========
    // x小于0 或者大于等于地图宽度；y同理，超出地图边界游戏结束
    if(new_head->x < 0 || new_head->x >= snake->width || new_head->y < 0 || new_head->y >= snake->height)
    {
        snake->game_over = 1;
        free(new_head);   // 一定要释放刚分配的新蛇头，否则内存泄漏
        return;
    }

    // =========撞到自身身体判定=========
    list_node_t *cur = snake->body->head;
    while(cur != NULL)
    {
        snake_body_t *body_part = (snake_body_t*)cur->data;
        // 新蛇头坐标和身体任意一节重合，撞到自己
        if(body_part->x == new_head->x && body_part->y == new_head->y)
        {
            snake->game_over = 1;
            free(new_head);
            return;
        }
        cur = cur->next;
    }

    // 把新蛇头插入链表头部，蛇向前长出一节
    list_push_front(snake->body, new_head);

    // =========判断是否吃到食物=========
    if(new_head->x == snake->food_x && new_head->y == snake->food_y)
    {
        // 吃到食物：不删除尾巴，蛇身体变长；立刻生成下一份随机食物
        snake_gen_food(snake);
        snake->snake_len++;
        snake->grade = snake_get_grade(snake->snake_len);
    }
    else
    {
        // 没有吃到食物：弹出链表尾部节点，蛇整体向前平移，总长度不变
        list_pop_back(snake->body);
    }
}

/**
 * @brief 随机生成食物坐标food_x food_y
 * do‑while循环：不断随机坐标，直到找到一个不在蛇身体上的合法位置
 * 食物不贴地图最边缘，x∈[1,width‑2]，y∈[1,height‑2]
 * @param snake 游戏实例
 */
void snake_gen_food(snake_t *snake)
{
    if(snake == NULL)
        return;
    int ok;
    do
    {
        ok = 1;   // ok=1代表当前随机出来的坐标合法，可以放食物
        //【修复】防止坐标越界，避开地图左右、上下边界
        snake->food_x = 1 + rand() % (snake->width - 2);
        snake->food_y = 1 + rand() % (snake->height - 2);

        // 遍历整条蛇，检查这个坐标是否踩在蛇身体上
        list_node_t *cur = snake->body->head;
        while(cur != NULL)
        {
            snake_body_t *body_part = (snake_body_t*)cur->data;
            if(body_part->x == snake->food_x && body_part->y == snake->food_y)
            {
                ok = 0; // 食物刷蛇身上，标记非法，需要重新随机
                break;
            }
            cur = cur->next;
        }
    } while(!ok);  // ok等于0，继续循环重新生成坐标
}