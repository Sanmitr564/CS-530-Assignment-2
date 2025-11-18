
struct Node {
    void* data;
    struct Node *nextNode;
};

typedef struct Node Node;

typedef struct{
    Node *head;
    Node *tail;
}List;

Node *newNode(data);

Node *createAndAppendNode(List *list, void* data);

void addNode(List *list, Node* node);
