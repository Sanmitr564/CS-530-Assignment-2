#include <stdlib.h>
#include <string.h>

#ifndef LINKEDLIST
#define LINKEDLIST
#include "linkedlist.h"
#endif

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

Node *createAndAppendNode(List *list, void* data){
    Node* node = newNode(data);
    addNode(list, node);
    return node;
}

void addNode(List *list, Node* node){
    if (list->head == NULL) {
        list->head = node;
        list->tail = node;
        return;
    }
    list->tail->nextNode = node;
    list->tail = node;
}
