
struct Node {
    void* data;
    struct Node *nextNode;
};

typedef Node Node;

typedef struct{
    Node *head;
    Node *tail;
}List;

Node newNode(data);

Node createAndAppendNode(List *list, void* data);

void addNode(List *list, Node* node);

/*
unsigned int address;
    char *label;
    char *opcode;
    char *operand;
    char *comment;
*/