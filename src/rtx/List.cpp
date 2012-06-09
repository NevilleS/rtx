#include "List.h"
#include "RTXConstants.h"
#include <stdio.h>
#include <stdlib.h>

// Add a generic object to the end of the list
template <class T>
int List<T>::enqueue(T* object) {
    // Validate input
    if(object == NULL) {
        return NULL_POINTER;
    }

    //Allocate a new ListNode for the object
    ListNode<T>* node = new ListNode<T>;
    if(node == NULL) {
        return OUT_OF_MEMORY; //malloc failed
    }
    node->_object = object;

    //Add the node to the list
    if(_head == NULL && _tail == NULL) {
        _head = node;
        _tail = node;
        node->_prev = NULL;
        node->_next = NULL;
    }
    else {
        _tail->_next = node;
        node->_prev = _tail;
        node->_next = NULL;
        _tail = node;
    }

    //Update the node count
    _count++;
    return SUCCESS;
}

// Remove the first node from the list and return its generic object
template <class T>
T* List<T>::dequeue() {
    // Check if the list is empty
    if(_head == NULL) {
        return NULL;
    }
    else {
        //Retrieve the first object and update the list
        ListNode<T>* node = _head;
        T* object = node->_object;
        if(node->_next == NULL) {
            _head = NULL;
            _tail = NULL;
        }
        else {
            _head = node->_next;
            _head->_prev = NULL;
        }
        _count--;
        node->_object = NULL;
        node->_prev = NULL;
        node->_next = NULL;
        delete node;

        return object;
    }
}

// Insert a new object into the list and sort it using an object-specific sort function
template <class T>
int List<T>::insert(T* object, T* (*sortFunction)(T* A, T* B)) {
    // Validate input
    if(object == NULL) {
        return NULL_POINTER;
    }

    //Allocate a new ListNode for the object
    ListNode<T>* node = new ListNode<T>;
    if(node == NULL) {
        return OUT_OF_MEMORY; //malloc failed
    }
    node->_object = object;

    //Add the node to the list
    if(_head == NULL && _tail == NULL) {
        _head = node;
        _tail = node;
        node->_prev = NULL;
        node->_next = NULL;
        return SUCCESS;
    }
    else {
        //Traverse the sorted list, looking for where to insert the node
        ListNode<T>* current = _head;
        while(current != NULL) {
            //Compare the new object with the current node's object
            T* greater = (*sortFunction)(current->_object, object);
            //If object is greater, insert node before current
            if(greater == object) {
                //Handle the case where node becomes _head
                if(current == _head) {
                    _head = node;
                }
                else {
                    node->_prev = current->_prev;
                    node->_prev->_next = node;
                }
                node->_next = current;
                current->_prev = node;
                _count++;
                return SUCCESS;
            }
            else {
                current = current->_next;
            }
        }
        //Append the node
        _tail->_next = node;
        node->_prev = _tail;
        node->_next = NULL;
        _tail = node;
        _count++;
        return SUCCESS;
    }
}

// Remove a specific generic object from the list
template <class T>
int List<T>::remove(T* object) {
    // Validate input
    if(object == NULL) {
        return NULL_POINTER;
    }
    // Traverse the list
    ListNode<T>* current = _head;
    while(current != NULL) {
        if(current->_object == object) {
            // Remove the node
            if(current == _head) {
                _head = current->_next;
                if(_head != NULL) {
                    _head->_prev = NULL;
                }
            }
            else if(current == _tail) {
                _tail = current->_prev;
                if(_tail != NULL) {
                    _tail->_next = NULL;
                }
            }
            else {
                current->_prev->_next = current->_next;
                current->_next->_prev = current->_prev;
            }
            _count--;
            current->_next = NULL;
            current->_prev = NULL;
            current->_object = NULL;
            delete current;
            return SUCCESS;
        }    
        current = current->_next;
    }
    // Could not find the object in the list
    return OUT_OF_BOUNDS;
}

template <class T>
T* List<T>::getHead() {
    if(_head != NULL) {
        return _head->_object;
    }
    return NULL;
}

template <class T>
T* List<T>::getTail() {
    if(_tail != NULL) {
        return _tail->_object;
    }
    return NULL;
}

