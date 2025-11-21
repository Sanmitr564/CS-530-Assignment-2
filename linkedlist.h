/*
Aaron Alegre,       cssc2101, Red ID: 828222103
Patrick Stewart,    cssc2138, Red ID: 130969511
Edwin Vega,         cssc2141, Red ID: 827746186
*/

struct Node {
    void* data;
    struct Node *nextNode;
};

typedef struct Node Node;

typedef struct{
    Node *head;
    Node *tail;
}List;

Node *newNode(void* data);

Node *createAndAppendNode(List *list, void* data);

void addNode(List *list, Node* node);
