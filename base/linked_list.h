#ifndef __LINKED_LIST_H__
#define __LINKED_LIST_H__
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Enumeration for linked list operation return status codes
 */
typedef enum{
	LIST_OK =             0,    // Operation completed successfully
	LIST_ERR_NULL =      -1,    // Null pointer passed as list argument
	LIST_ERR_MALLOC =    -2,    // Memory allocation failed (malloc returned NULL)
	LIST_ERR_EMPTY =     -3,    // List is empty, cannot perform remove operation
	LIST_ERR_NOT_FOUND = -4,    // Target element not found in list
	LIST_ERR_NOT_EMPTY = -5,    // List is not empty when checked
	LIST_ERR_INDEX =     -6,    // Insert position index out of valid range
}list_state_t;

/**
 * @brief Single linked list node structure
 * @param data Generic void pointer to store user-defined data
 * @param next Pointer pointing to the next list node
 */
typedef struct list_node{
        void   *data;
        struct list_node *next;
}list_node_t;

/**
 * @brief Linked list control header structure
 * @param head Pointer to the first node of the list
 * @param tail Pointer to the last node of the list
 * @param size Total count of valid nodes stored in list
 * @param data_free Callback function to release user allocated data
 */
typedef struct {
        list_node_t *head;
        list_node_t *tail;
        uint32_t     size;
        void (*data_free)(void* data);
}list_t;

/**
 * @brief Callback function type for list traversal
 * @param data User data stored inside current iterated node
 * @return int Return 0 to continue traversal, non-zero to break immediately
 */
typedef int (*list_foreach_cb)(void *data,uint32_t index,void *arg);

/**
 * @brief Callback function type for data comparison during search
 * @param node_data Data stored in the current list node
 * @param user_data User-provided data for comparison
 * @return int Return 0 if nodes are equal, non-zero otherwise
 */
typedef int (*list_cmp_cb)(void *node_data, void *user_data);

/**
 * @brief Create an empty linked list control header
 * @param data_free Custom callback to free user data; pass NULL if no need to release data
 * @return list_t* Return valid list pointer on success, NULL if malloc fails
 */
list_t *list_create(void (*data_free)(void *data));

/**
 * @brief Append a new node at the end of the list
 * @param list Pointer of linked list control header
 * @param data User data pointer to store inside new node
 * @return list_state_t Operation status code
 */
list_state_t list_push_back(list_t *list ,void *data);

/**
 * @brief Prepend a new node at the head of the list
 * @param list Pointer of linked list control header
 * @param data User data pointer to store inside new node
 * @return list_state_t Operation status code
 */
list_state_t list_push_front(list_t *list,void* data);

/**
 * @brief Insert new node at specified index position
 * @param list Pointer of linked list control header
 * @param pos Insert index starting from 0; pos equal to list size means append to tail
 * @param data User data pointer to store inside new node
 * @return list_state_t Operation status code
 */
list_state_t list_insert(list_t *list,uint32_t pos,void *data);

/**
 * @brief Remove node at specified index position
 * @param list Pointer of linked list control header
 * @param pos Remove index starting from 0
 * @return list_state_t Operation status code
 */
list_state_t list_remove(list_t *list,uint32_t pos);

/**
 * @brief Remove the first node at list head
 * @param list Pointer of linked list control header
 * @return list_state_t Operation status code
 */
list_state_t list_pop_front(list_t *list);

/**
 * @brief Remove the last node at list tail
 * @param list Pointer of linked list control header
 * @return list_state_t Operation status code
 */
list_state_t list_pop_back(list_t *list);

/**
 * @brief Traverse every node and execute user callback function
 * @param list Pointer of linked list control header
 * @param cb User defined traversal callback function
 * @return list_state_t Operation status code
 */
list_state_t list_foreach(list_t *list,list_foreach_cb cb);

/**
 * @brief Reverse all nodes of linked list in-place
 * @param list Pointer of linked list control header
 * @return list_state_t Operation status code
 */
list_state_t list_reverse(list_t *list);

/**
 * @brief Find the first node matching user comparison callback
 * @param list Pointer of linked list control header
 * @param user_data User-provided data passed to comparison callback
 * @param cmp Comparison callback function; return 0 when equal
 * @param pos Output pointer to receive the found node index
 * @return list_state_t Operation status code
 */
list_state_t list_find(list_t *list, void *user_data, list_cmp_cb cmp, uint32_t *pos);

/**
 * @brief Retrieve stored data pointer from a node by index
 * @param list Pointer of linked list control header
 * @param pos Node index starting from 0
 * @return void* Stored data pointer or NULL if invalid index
 */
void *list_get(list_t *list,uint32_t pos);

/**
 * @brief Replace stored data in a node by index
 * @param list Pointer of linked list control header
 * @param pos Node index starting from 0
 * @param data New user data pointer to assign
 * @return list_state_t Operation status code
 */
list_state_t list_set(list_t *list,uint32_t pos,void *data);

/**
 * @brief Check whether the list is empty
 * @param list Pointer of linked list control header
 * @return list_state_t LIST_OK when empty, LIST_ERR_NOT_FOUND when not empty, LIST_ERR_NULL on null list
 */
list_state_t list_check(list_t *list);

/**
 * @brief Get current number of stored nodes
 * @param list Pointer of linked list control header
 * @return uint32_t Node count, or 0 if list pointer is NULL
 */
uint32_t list_size(list_t *list);

/**
 * @brief Clear all nodes but reserve list control header memory
 * @param list Pointer of linked list control header
 * @return list_state_t Operation status code
 */
list_state_t list_clear(list_t *list);

/**
 * @brief Destroy entire linked list, release all nodes and list header
 * @param list Pointer of linked list control header
 * @return list_state_t Operation status code
 */
list_state_t list_destroy(list_t *list);

#endif

