#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include ".\PythonListReplication.h"

/**
 * @brief The complement function for findData, performing comparing of indices
 * 
 * @param iterator The iterator passed by findData after adjusted to the unoffset part
 * @param data The given data
 * @param offset The previous offset, working as accumulative element for return value
 * @return int On success, the function will return the index of the given data, else -1 is return if error
 */
static int iterateFunc(list_element *iterator, const data* data, const int offset){
    int counter = offset;

    while (iterator != NULL){
        if (iterator->mode == data->mode){
            if (data->mode == __STRING__){
                if (!strcmp((char*)data->data, (char*)iterator->data))
                    return counter;
            }else{
                int ret = memcmp(data->data, iterator->data, iterator->size);

                if (!ret)
                    return counter;
            }
        }
        counter++;
        iterator = iterator->next;
    }
    return -1;
}

/**
 * @brief This function is a complement for the clearList function, clear the root in detail
 * 
 * @param root The root passed by the clearList function
 */
void freeListElement(list_element *root){
    list_element *iterator;
    
    while (root != NULL){
        iterator = root;
        root = root->next;

        free(iterator->data);
        free(iterator);
    }

    root = NULL;
}
/**
 * @brief This function will find the index of a given data structure which is inputted through arguments
 * 
 * @param arg_list The pointer to the list of checking
 * @param data The given data
 * @param offset The offset of checking process, the iterator will skipped to the unoffset part
 * @return int On success, the function will return the index of the given data, else -1 is return if error
 */
int findData(list ** arg_list, const data* data, const int offset){
    if (data == NULL || (data->mode < 0 || data->mode > 12)){
        printf("Data is null or invalid mode!\n");
        goto fail;
    }

    if (offset < 0 || offset >= (*arg_list)->size){
        printf("Invalid offset, offset value larger than the size of list!\n");
        goto fail;
    }

    list_element *iterator = (*arg_list)->root;

    int offsetCount = 0;
    while (iterator != NULL && offsetCount < offset){
        iterator = iterator->next;
        offsetCount++;
    }

    if (!iterator)
        goto fail;

    return iterateFunc(iterator, data, offset);
    
    fail:
    return -1;
}

/**
 * @brief This function will return the data structure based on the given index
 * 
 * @param arg_list The pointer to the list of checking
 * @param pos The position of index
 * @return data* On sucess, the pointer to this data structure will be returned, else NULL will be returned
 */
data* findIndex(list** arg_list, const int pos){
    if (*arg_list == NULL){
        printf("List is clear, try to add more elements (getIndex)\n");
        goto fail;
    }else{
        list_element* iterator = (*arg_list)->root;

        if (iterator->next == NULL){
            return parseData(iterator->size, iterator->data, iterator->mode);
        }else{
            if (pos < 0){
                while (iterator->next != NULL)
                    iterator = iterator->next;
            }else{
                int counter = 0;

                while (counter != pos && iterator->next != NULL){
                    iterator = iterator->next;
                    counter++;
                }
            }
        }
        return parseData(iterator->size, iterator->data, iterator->mode);
    }
    fail:
    return NULL;

}

/**
 * @brief This function will parse the given content into the data structure
 * 
 * @param size The total size of the content (must be converted in bytes)
 * @param content The void pointer to the given content
 * @param mode The data type (should be setted with defined macro) of the content
 * @return data* On sucess, the pointer to this data structure will be returned, else NULL will be returned
 */
data* parseData(const int size, void* content, const int mode){
    if (mode < 0 || mode > 12){
        printf("Invalid mode, try to use defined macro instead!\n");
        goto fail;
    }

    data* ret = (data*)malloc(sizeof(data));
    
    ret->size = size;
    ret->data = malloc(sizeof(size));

    if(ret->data)
        memcpy(ret->data, content, size);
    else{
        printf("Failed to allocate memory! (Initialize process)\n");
        goto fail;
    }
    
    ret->mode = mode;

    return ret;

    fail:
    return NULL;
}

/**
 * @brief This function will add multiple data structure into the list (it is encouraged to used with simultanenous insertion, like initialization)
 * 
 * @param arg_list The pointer to the list of checking
 * @param ... Data structures, must be ended with macro "__LIST_END__"
 */
void makeList(list** arg_list, ...){
    va_list list_va;

    va_start(list_va, arg_list);

    while (1){
        data* ret = va_arg(list_va, data*);

        if (ret == __LIST_END__)
            break;

        addIndex(arg_list, ret->data, ret->size, -1, ret->mode);
    }

    va_end(list_va);   
}

/**
 * @brief This function will free the whole list
 * 
 * @param arg_list The pointer to the list to be freed
 */
void clearList(list **arg_list){
    freeListElement((*arg_list)->root);
    free(*arg_list);
    *arg_list = NULL;
}



/**
 * @brief This function will delete a list_element structure at a given index
 * 
 * @param arg_list The pointer to the list to be deleted
 * @param pos The given index to the list_element structure to be deleted. Negative values are treated as end of list;
 * @return int On success, the function returns 1, else -1 will be returned
 */
int delete(list** arg_list, const int pos){
    
    if (*arg_list == NULL){
        printf("List is clear, try to add elements! (delete)\n");
        goto fail;
    }else{
        list_element* iterator = (*arg_list)->root;

        if (iterator->next == NULL){
                //FREE THE LIST;
                clearList(arg_list);
        }else{
            if (pos < 0){
                while (iterator->next->next != NULL)
                    iterator = iterator->next;

                free(iterator->next->data);
                free(iterator->next);
                iterator->next = NULL;

            }else if(pos == 0){
                list_element *buffer = (*arg_list)->root;
                (*arg_list)->root        = (*arg_list)->root->next;

                free(buffer);
            }else{
                int counter = 0;

                while (counter != pos-1 && iterator->next->next != NULL){
                    iterator = iterator->next;
                    counter++;
                }

                list_element *buffer =  iterator->next->next;
                free(iterator->next->data);
                free(iterator->next);

                iterator->next = NULL;
                iterator->next = buffer;


            }
            (*arg_list)->size--;
        }
    }
    return 1;
    fail:
    return -1;

}

/**
 * @brief This function will add a new node based on the given index
 * 
 * @param arg_list The pointer to the list to be inserted
 * @param value The pointer to the value
 * @param size The size of the value
 * @param pos The position to be added, input negative value to add at the end
 * @param data_type The defined macro of data_type
 * @return int On sucesss, 0 will be returned, else 1 will be returned if error
 */
int addIndex(list** arg_list, const void* value, const size_t size, const int pos, const int data_type){
    if (data_type < 0 || data_type > 12){
        printf("Invalid data type, try to use defined macro instead!\n");
        goto fail;
    }
    if ((*arg_list)== NULL){
        //Initialize the list
        *arg_list         = (list*)malloc(sizeof(list));
        (*arg_list)->root = (list_element*)malloc(sizeof(list_element));

        (*arg_list)->root->data = malloc(size);

        if ((*arg_list)->root->data)
            memcpy((*arg_list)->root->data, value, size);
        else{
            printf("Failed to allocate memory! (Initialize process)\n");
            goto fail;
        }
        (*arg_list)->root->size = size;
        (*arg_list)->root->next = NULL;
        (*arg_list)->root->mode = data_type;
        (*arg_list)->size = 1;
    }else{

        list_element* new_list_element = (list_element*)malloc(sizeof(list_element));
        new_list_element->data = malloc(size);
        new_list_element->size = size;
        new_list_element->mode = data_type;

        if (new_list_element->data)
            memcpy(new_list_element->data, value, size);
        else{
            printf("Failed to allocate memory (insertion process)!\n");
            goto fail;
        }

        if (pos == 0){
            new_list_element->next = (*arg_list)->root;
            (*arg_list)->root = new_list_element;
        }else if (pos < 0){
            list_element *iterator = (*arg_list)->root;
            while(iterator->next != NULL)
                iterator = iterator->next;

            iterator->next = new_list_element;
            new_list_element->next = NULL;
        }else{
            int counter = 0;
            list_element *iterator = (*arg_list)->root;
            list_element *buffer;

            while (counter != pos && iterator->next != NULL){
                iterator = iterator->next;
                counter++;
            }
            
            buffer = iterator->next;
            iterator->next = new_list_element;
            new_list_element->next = buffer;
        }
        (*arg_list)->size++;
    }
    return EXIT_SUCCESS;
    fail:
    return EXIT_FAILURE;
}

/**
 * @brief This built-in print function will print out the list, bypass error and custom input type
 * 
 * @param arg_list The pointer to the list to be printed
 * @param step The iteration cycles 
 */
void print(list **arg_list, const int step){
    if(*arg_list == NULL){
        printf("[]\n");
        return;
    }

    if (!step){
        printf("Invalid step, step musn't be 0");
        return;
    }
    
    list_element *iterator = (*arg_list)->root;
    printf("[");
    while (iterator != NULL){
        switch(iterator->mode){
            case __SHORT__:
                printf("%i", *((short*)iterator->data));
                break;
            case __UNSIGNED_SHORT:
                printf("%i", *((unsigned short*)iterator->data));
                break;
            case __CHAR__:
                printf("%c", *((char*)iterator->data));
                break;
            case __UNSIGNED_CHAR__:
                printf("%c", *((unsigned char*)iterator->data));
                break;
            case __INT__:
                printf("%d", *((int*)iterator->data));
                break;
            case __UNSIGNED_INT__:
                printf("%lu", *((unsigned int*)iterator->data));
                break;
            case __LONG__:
                printf("%li", *((long*)iterator->data));
                break;
            case __UNSIGNED_LONG__:
                printf("%lu", *((unsigned long*)iterator->data));
                break;
            case __FLOAT__:
                printf("%f", *((float*)iterator->data));
                break;
            case __DOUBLE__:
                printf("%lf", *((double*)iterator->data));
                break;
            case __LONG_DOUBLE__:
                printf("%Lf", *((long double*)iterator->data));
                break;
            case __STRING__:
                printf("%s", ((char*)iterator->data));
                break;
            case __CUSTOM__:
                printf("CUSTOM");
                break;
            default:
                printf("ERROR");
        }

        for (int i = 0; i < step; i++){
            if (iterator->next == NULL){
                printf("]\n");
                return;
            }

            iterator = iterator->next;
        }

        if (iterator != NULL)
            printf(", ");
    }
    printf("]\n");

}