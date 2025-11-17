#include <stdlib.h>
#include <string.h>

#include "linkedlist.h"

Node newNode(void* data){
    Node* node = (Node*)malloc(sizeof(Node));

    node->data = data;
    node->nextNode = NULL;
    return node;
}

Node createAndAppendNode(List *list, void* data){
    Node* node = newNode(data);
    addnode(list, node);
    return node;
}

void addNode(List *list, Node* node){
    list->tail->nextNode = node;
    list->tail = node;
}
