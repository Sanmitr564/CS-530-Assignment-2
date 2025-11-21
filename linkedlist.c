#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifndef LINKEDLIST
#define LINKEDLIST
#include "linkedlist.h"
#endif

//create a new node
Node *newNode(void* data){
    Node* node = (Node*)malloc(sizeof(Node));

    if (node == NULL) {
        printf("Could not allocate memory for node.\n");
        exit(9);
    }

    node->data = data;
    node->nextNode = NULL;
    return node;
}

//create and append node to list
Node *createAndAppendNode(List *list, void* data){
    Node* node = newNode(data);
    addNode(list, node);
    return node;
}

//add node to list
void addNode(List *list, Node* node){
    if (list->head == NULL) {
        list->head = node;
        list->tail = node;
        return;
    }
    list->tail->nextNode = node;
    list->tail = node;
}
