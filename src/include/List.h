#ifndef List_H
#define List_H

#include <stdlib.h>

//Define the generic template class
template <class T>
class ListNode {
    public:
        ListNode* _prev; 
        ListNode* _next; 
        T* _object;
};

template <class T>
class List {
    private:
        // List pointers and node count
        ListNode<T>* _head;
        ListNode<T>* _tail;
        int _count;

    public:
        // Initializes variables
        List() {
            _head = NULL;
            _tail = NULL;
            _count = 0;
        };
        ~List() {
            ListNode<T>* current = _head;
            while(current != NULL) {
                _head = current->_next;
                delete current;
                current = _head;
            }
        }
 
        /***
         * Creates a new node to store the generic object and appends the node to the list.
         * In combination with dequeue(), this provides FIFO QUEUE functionality.
         *
         * Params:
         *    object: pointer to the generic object to be stored
         *
         * Returns:
         *    integer error code
         ***/
        int enqueue(T* object);

        /***
         * Removes the first node from the list, and returns the generic object it contains.
         * The second half of FIFO QUEUE functionality.
         *
         * Params:
         *    none
         *
         * Returns:
         *    pointer to the generic object the first node contained, or NULL if the list is empty
         ***/
        T* dequeue();

        /***
         * Inserts a new object into the sorted list based on the logic provided by the sort function. The list
         * is maintained in descending order.
         *
         * Params:
         *   object: pointer to the generic object to be stored
         *   sortFunction: pointer to a sort function that returns the greater of two objects. During insertion,
         *                 the list is traversed in forward order, calling this function at each step, providing
         *                 the current object as parameter A and the object to be inserted as B. If the pointer
         *                 returned is B, the object is inserted just before A in the list, and the traversal stops.
         *
         * Returns:
         *   integer error code
         ***/
        int insert(T* object, T* (*sortFunction)(T* A, T* B));

        /***
         * Removes an object from the list.
         *
         * Params:
         *   object: pointer to the generic object to be removed
         *
         * Returns:
         *   integer error code
         ***/
        int remove(T* object);

        /***
         * Retrieve information about the List.
         *
         * Params:
         *   none
         *
         * Returns:
         *   self-explanatory information about the list, head and tail are null if empty.
         ***/
        T* getHead();
        T* getTail();
        int getCount() { return _count; }

};

#endif

