#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum 
{
    TK_RESERVED, // 記号
    TK_NUM,      // 整数トークン
    TK_EOF       // 入力の終わりを表すトークン
} TokenKind;

// 抽象構文木のノードの種類
typedef enum
{
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_EQ,  // ==
    ND_NE,  // !=
    ND_LT,  // <
    ND_LE,  // <=
    ND_GT,  // >
    ND_GE,  // >=
    ND_NUM, // 整数
} NodeKind;

typedef struct Token Token;

struct Token
{
    TokenKind kind; // トークンの型
    Token *next;    // 次の入力トークン
    int val;        // kindがTK_NUMの場合、その数値
    char *str;      // トークン文字列
    int len;        // トークンの長さ
};

typedef struct Node Node;

// 抽象構文木のノードの型
struct Node
{
    NodeKind kind; // ノードの型
    Node *lhs; // 左辺 left-hand side
    Node *rhs; // 右辺 right-hand side
    int val; // kindがND_NUMの場合のみ使う
};

char *user_input;

// パーサが読み込むトークン列。連結リストになっているtokenを辿っていく
Token *token;

// エラー箇所を報告する
void error_at(char *loc, char *fmt, ...)
{
    va_list ap;
    // va_listの初期化
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    // pos個の空白を出力
    fprintf(stderr, "%*s", pos, " "); 
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// エラーを報告するための関数
void error(char *fmt, ...) 
{
    va_list ap;
    // va_listの初期化
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// 次のトークンが求めている記号のときには、トークンを一つ読み進め、真を返す
bool consume(char *op)
{
    if (token->kind != TK_RESERVED || 
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len))
        return false;
    token = token->next;
    return true;
}

// 次のトークンが求める記号のときは、トークンを一つ読み進める
// それ以外はエラーを返す
void expect(char *op)
{
    if (token->kind != TK_RESERVED || 
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len)) {
        error_at(token->str, "\"%s\"ではありません", op);
    } else {
        token = token->next;
    }
}

// 次のトークンが数値の場合、トークンを一つ読み進めてその数値を返す
int expect_number()
{
    if (token->kind != TK_NUM) {
        error_at(token->str, "数ではありません");
    } else {
        int val = token->val;
        token = token->next;
        return val;
    }
}

bool at_eof()
{
    return token->kind == TK_EOF;
}

// 新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str, int len)
{
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

bool startswith(char *p, char *q)
{
    return memcmp(p, q, strlen(q)) == 0;
}

// 文字列をトークナイズして返す
// 連結リストを構築
Token *tokenize(char *p)
{
    // ダミーのhead要素を作る
    Token head;
    head.next = NULL;
    // ダミーのhead要素に新しい要素を繋げていく
    Token *cur = &head;

    while (*p)
    {
        if (isspace(*p)) {
            p++;
            continue;
        }

        if (startswith(p, "==") || startswith(p, "!=") ||
            startswith(p, "<=") || startswith(p, ">=")) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue; 
        }

        if (strchr("+-*/()<>", *p)) {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p, 0);
            char *q = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }

        error_at(p, "トークナイズできません");
    }

    new_token(TK_EOF, cur, p, 0);
    return head.next;
}

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

Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

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

void gen(Node *node)
{
    if (node->kind == ND_NUM) {
        printf("  mov x0, #%d\n", node->val);
        // アセンブリの"!"はregister write-backを意味する。以下の場合はsp=sp-16となる。
        printf("  str x0, [sp, #-16]!\n");
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("  ldr x1, [sp], #16\n");
    printf("  ldr x0, [sp], #16\n");

    switch (node->kind)
    {
    case ND_ADD:
        printf("  add x0, x0, x1\n");
        break;
    case ND_SUB:
        printf("  sub x0, x0, x1\n");
        break;
    case ND_MUL:
        printf("  mul x0, x0, x1\n");
        break;
    case ND_DIV:
        printf("  sdiv x0, x0, x1\n");
        break;
    case ND_EQ:
        printf("  cmp x0, x1\n");
        printf("  cset x0, EQ\n");
        break;
    case ND_NE:
        printf("  cmp x0, x1\n");
        printf("  cset x0, NE\n");
        break;
    case ND_LT:
        printf("  cmp x0, x1\n");
        printf("  cset x0, LT\n");
        break;
    case ND_LE:
        printf("  cmp x0, x1\n");
        printf("  cset x0, LE\n");
        break;
    case ND_GT:
        printf("  cmp x0, x1\n");
        printf("  cset x0, GT\n");
        break;
    case ND_GE:
        printf("  cmp x0, x1\n");
        printf("  cset x0, GE\n");
        break;
    }

    printf("  str x0, [sp, #-16]!\n");
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "引数の個数が正しくありません。\n");
        return 1;
    }

    // トークナイズしてパースする
    user_input = argv[1];
    token = tokenize(user_input);
    Node *node = expr();

    printf(".text\n");
    printf(".global main\n");
    printf("main:\n");

    // 抽象構文木を下りながらコード生成
    gen(node);
    
    printf("  ldr x0, [sp], #16\n");
    printf("  mov x8, #93\n");
    printf("  svc #0\n");
    return 0;
}