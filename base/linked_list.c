#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "linked_list.h"

/**
 * @brief Internal static function: Allocate and initialize a single list node
 * @param data User data pointer to bind to new node
 * @return list_node_t* Valid node pointer on success, NULL if malloc failed
 */
static list_node_t *list_node_create(void *data)
{
	list_node_t *node = (list_node_t*)malloc(sizeof(list_node_t));
	if(node == NULL)           return NULL; 
	node->data = data;
	node->next = NULL;
	return node;
}

list_t *list_create(void (*data_free)(void *data))
{	
	list_t *list = (list_t*)malloc(sizeof(list_t));
	if(list == NULL)          return NULL; 
	list->head = NULL;
	list->tail = NULL;
	list->size = 0;
	list->data_free = data_free;
	return list;
}

list_state_t list_push_back(list_t *list ,void *data)
{
	if(list == NULL)          return LIST_ERR_NULL; 
	list_node_t *new_node = list_node_create(data);
	if(new_node == NULL)      return LIST_ERR_MALLOC; 
	
	if(list->head == NULL)
	{
		// Empty list, both head and tail point to new node
		list->head = new_node;
		list->tail = new_node;
	}
	else 
	{
		// Link old tail to new node, update tail pointer
		list->tail->next = new_node;
		list->tail = new_node;
	}
	list->size++;
	return LIST_OK ;
}

list_state_t list_push_front(list_t *list,void* data)
{
	if(list == NULL)          return LIST_ERR_NULL; 
	list_node_t *new_node = list_node_create(data);
	if(new_node == NULL)      return LIST_ERR_MALLOC; 
	// New node links to original head node
	new_node->next = list->head;
	// Update list head pointer to new node
	list->head = new_node;          
	// If list was empty, synchronize tail pointer
	if(list->tail == NULL)    list->tail = new_node; 
	list->size++;
	return LIST_OK;
}

list_state_t list_insert(list_t *list,uint32_t pos,void *data)
{
	if(list == NULL)          return LIST_ERR_NULL; 
	// Check index overflow: maximum valid pos is list->size
	if(pos > list->size)      return LIST_ERR_INDEX; 
	// Insert at head, reuse push front logic
	if(pos == 0)              return list_push_front(list,data); 
	// Insert at tail, reuse push back logic
	if(pos == list->size)     return list_push_back(list,data);
	// Insert between two nodes
	list_node_t *new_node = list_node_create(data);
    if(new_node == NULL)      return LIST_ERR_MALLOC;
	// Locate previous node of target position
	list_node_t *prev = list->head;
	for(uint32_t i = 0; i < pos - 1;i++)		prev = prev->next;
	// Connect new node with original next node
	new_node->next = prev->next;
	// Link previous node to new inserted node
	prev->next = new_node;
	list->size++;

	return LIST_OK;
}

list_state_t list_remove(list_t *list,uint32_t pos)
{
	if(list == NULL)           return LIST_ERR_NULL;
	if(list->head == NULL)     return LIST_ERR_EMPTY;
	if(pos >= list->size)      return LIST_ERR_INDEX;

	if(pos == 0)               return list_pop_front(list);
	if(pos == list->size - 1)  return list_pop_back(list);

	list_node_t *prev = list->head;
	for(uint32_t i = 0; i < pos - 1; i++)
	{
		prev = prev->next;
	}
	list_node_t *tmp = prev->next;
	prev->next = tmp->next;
	if(list->data_free != NULL)
	{
		list->data_free(tmp->data);
	}
	free(tmp);
	list->size--;
	return LIST_OK;
}

list_state_t list_pop_front(list_t *list)
{
	if(list == NULL)
    {
        return LIST_ERR_NULL;
    }
	// Cannot remove node from empty list
	if(list->head == NULL)    return LIST_ERR_EMPTY;
	
	list_node_t *tmp_node = list->head;
	// Move head pointer to next node
	list->head = list->head->next;
	// Release user allocated data via registered callback
	if(list->data_free != NULL)		list->data_free(tmp_node->data);
	free(tmp_node);
	// If list becomes empty after remove, reset tail pointer to NULL
	if(list->head == NULL)		list->tail = NULL;
	list->size--;
	return LIST_OK;
}

list_state_t list_pop_back(list_t *list)
{
	if(list == NULL)          return LIST_ERR_NULL;
	if(list->head == NULL)    return LIST_ERR_EMPTY;
	// Only one single node exists in list
	if(list->head == list->tail)
	{
		if(list->data_free != NULL)   list->data_free(list->head->data);
		free(list->head);
		list->head = NULL;
		list->tail = NULL;
	}
	else
	{
		// Traverse to get second-last node
		list_node_t *prev = list->head;
		for(uint32_t i = 0;i < list->size - 2;i++)
		{
			prev = prev->next;
		}
		// Save target tail node to delete
		list_node_t *tmp_node = list->tail;
		if(list->data_free != NULL)  list->data_free(tmp_node->data);
		free(tmp_node);
		// Second-last node becomes new tail, terminate link
		prev->next = NULL;
		list->tail = prev;
	}
	list->size--;
	return LIST_OK;
}

list_state_t list_foreach(list_t *list,list_foreach_cb cb)
{
	if(list == NULL)         return LIST_ERR_NULL;
	if(cb == NULL)           return LIST_ERR_NULL;

	list_node_t *cur = list->head;
	uint32_t index = 0;
	while(cur != NULL)
	{
		// Invoke user defined callback with node data, index and optional argument
		int ret = cb(cur->data, index, NULL);
		// Break traversal if callback returns non-zero value
		if(ret != 0)
		{
			break;
		}
		cur = cur->next;
		index++;
	}
	return LIST_OK;
}

list_state_t list_reverse(list_t *list)	
{
	if(list == NULL)         return LIST_ERR_NULL;
	// No need reverse empty list
	if(list->head == NULL)   return LIST_OK;
	// Save original head, it will become new tail after reverse
	list_node_t *old_head = list->head;
	list_node_t *prev = NULL;
	list_node_t *cur = list->head;
	while(cur != NULL)
	{
		// Backup next node before reverse pointer
		list_node_t *next = cur->next;
		// Reverse current node's link direction
		cur->next = prev;
		// Shift pointers forward
		prev = cur;
		cur = next;
	}
	// Refresh list head and tail pointer
	list->tail = old_head;
	list->head = prev;
	return LIST_OK;
}

list_state_t list_find(list_t *list, void *user_data, list_cmp_cb cmp, uint32_t *pos)
{
	if(list == NULL || pos == NULL || cmp == NULL)
		return LIST_ERR_NULL;
	if(list->head == NULL)
		return LIST_ERR_EMPTY;

	list_node_t *cur = list->head;
	uint32_t index = 0;
	while(cur != NULL)
	{
		if(cmp(cur->data, user_data) == 0)
		{
			*pos = index;
			return LIST_OK;
		}
		cur = cur->next;
		index++;
	}
	return LIST_ERR_NOT_FOUND;
}


void *list_get(list_t *list,uint32_t pos)
{
	if(list == NULL)           return NULL;
	if(list->head == NULL)     return NULL;
	if(pos >= list->size)      return NULL;

	list_node_t *cur = list->head;
	for(uint32_t i = 0;i < pos;i++)      cur = cur->next;
	return cur->data;
}

list_state_t list_set(list_t *list,uint32_t pos,void *data)
{
	if(list == NULL)           return LIST_ERR_NULL;
	if(pos >= list->size)      return LIST_ERR_INDEX;

	list_node_t *cur = list->head;
	for(uint32_t i = 0; i < pos; i++)
	{
		cur = cur->next;
	}
	// Release original data if user provided a free callback
	if(list->data_free != NULL)
	{
		list->data_free(cur->data);
	}
	cur->data = data;
	return LIST_OK;
}

list_state_t list_check(list_t *list)
{
	if(list == NULL)           return LIST_ERR_NULL;
	return (list->size == 0) ? LIST_OK : LIST_ERR_NOT_FOUND;
}

uint32_t list_size(list_t *list)
{
	if(list == NULL)           return 0;
	return list->size;
}

/**
 * @brief Clear all data nodes, keep list control header
 */
list_state_t list_clear(list_t *list)
{
	if(list == NULL)         return LIST_ERR_NULL;

	list_node_t *cur = list->head;
	while(cur != NULL)
	{
		list_node_t *tmp = cur;
		cur = cur->next;
		if(list->data_free)
			list->data_free(tmp->data);
		free(tmp);
	}
	list->head = NULL;
	list->tail = NULL;
	list->size = 0;
	return LIST_OK;
}

/**
 * @brief Release entire list memory including control header
 */
list_state_t list_destroy(list_t *list)
{
	if(list == NULL)          return LIST_ERR_NULL;
	list_clear(list);
	free(list);
	return LIST_OK;
}

