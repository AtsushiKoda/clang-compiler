#include "64cc.h"

Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

// 新しいノードを作成する関数
Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

// 新しいノードを作成する関数
Node *new_node_number(int val)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

// 左結合の演算子をパースする関数
// 生成規則: expr = equality
Node *expr()
{
    Node *node = equality();
}

// 生成規則: equality = relational ("==" relational | "!=" relational)*
Node *equality()
{
    Node *node = relational();
    
    for (;;) {
        if (consume("=="))
            node = new_node(ND_EQ, node, relational());
        else if (consume("!="))
            node = new_node(ND_NE, node, relational());
        else
            return node;
    }
}

// 生成規則: relational = add ("<" add | "<=" add | ">" add |)
Node *relational()
{
    Node *node = add();

    for (;;) {
        if (consume("<"))
            node = new_node(ND_LT, node, add());
        else if (consume("<="))
            node = new_node(ND_LE, node, add());
        else if (consume(">"))
            node = new_node(ND_GT, node, add());
        else if (consume(">="))
            node = new_node(ND_GE, node, add());
        else
            return node;
    }
}

// 生成規則: add = mul ("+" mul | "-" mul)*
Node *add()
{
    Node *node = mul();

    for (;;) {
        if (consume("+"))
            node = new_node(ND_ADD, node, mul()); 
        else if (consume("-"))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }
}

// 生成規則: mul = unary ("*" unary | "/" unary)*
Node *mul()
{
    Node *node = unary();

    for (;;) {
        if (consume("*"))
            node = new_node(ND_MUL, node, unary()); 
        else if (consume("/"))
            node = new_node(ND_DIV, node, unary());
        else
            return node;
    }
}

// 生成規則: unary = ("+" | "-")? primary
Node *unary()
{
    if (consume("+"))
        return primary();
    if (consume("-"))
        // -x を 0-x に置き換える
        return new_node(ND_SUB, new_node_number(0), primary());
    return primary();
}

// 生成規則: primary = "(" expr ")" | num
Node *primary()
{
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }

    return new_node_number(expect_number());
}