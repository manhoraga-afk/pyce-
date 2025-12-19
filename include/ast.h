#ifndef AST_H
#define AST_H
// ADD TO NodeType ENUM:
NODE_BINARY,
NODE_UNARY_MINUS

// ADD THESE STRUCTS:
typedef enum {
    OP_PLUS,
    OP_MINUS,
    OP_TIMES,
    OP_DIVIDE
} OperatorType;

// ADD TO THE union in ASTNode:
struct {
    struct ASTNode* left;
    struct ASTNode* right;
    OperatorType op;
} binary;
struct {
    struct ASTNode* operand;
} unary;

typedef enum {
    NODE_SET,
    NODE_REPORT,
    NODE_JOIN,
    NODE_TONUMBER,
    NODE_TOTEXT,
    NODE_IDENTIFIER,
    NODE_LITERAL,
    NODE_ACTION_DEF,
    NODE_ACTION_CALL,
    NODE_IF,
    NODE_LOOP,
    NODE_EXIT,
    NODE_SKIP,
    NODE_CONTAIN,
    NODE_WAIT,
    NODE_BLOCK
} NodeType;

typedef struct ASTNode {
    NodeType type;
    union {
        struct {
            char name[64];
            struct ASTNode* value;
        } set;
        struct {
            struct ASTNode* expr;
        } report;
        struct {
            char ident[64];
            struct ASTNode* args;
            int count;
        } call;
        char ident[64];
        char literal[256];
        struct {
            struct ASTNode* cond;
            struct ASTNode* then_block;
            struct ASTNode* else_block;
        } if_stmt;
        struct {
            struct ASTNode* condition;
            struct ASTNode* body;
        } loop;
        struct {
            struct ASTNode* body;
        } contain;
        struct {
            struct ASTNode* condition;
        } wait;
    };
    struct ASTNode* next;
} ASTNode;

ASTNode* new_node(NodeType type);
void free_ast(ASTNode* node);

#endif