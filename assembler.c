typedef struct IntermediateRep{
    unsigned int address;
    char *label;
    char *opcode;
    char *operand;
    char *comment;
};

typedef struct SymtabEntry{
    char *csect;
    char *symbol;
    unsigned int value;
    unsigned int length;
    char *flags;
};

typedef struct LitTabEntry{
    char *name;
    int operand;
    int address;
    int length;
};