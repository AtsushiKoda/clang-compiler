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

typedef struct Token Token;

struct Token
{
    TokenKind kind; // トークンの型
    Token *next;    // 次の入力トークン
    int val;        // kindがTK_NUMの場合、その数値
    char *str;      // トークン文字列
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
bool consume(char op)
{
    if (token->kind != TK_RESERVED || token->str[0] != op) {
        return false;
    } else {
        token = token->next;
        return true;
    }
}

// 次のトークンが求める記号のときは、トークンを一つ読み進める
// それ以外はエラーを返す
void expect(char op)
{
    if (token->kind != TK_RESERVED || token->str[0] != op) {
        error_at(token->str, "'%c'ではありません", op);
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
Token *new_token(TokenKind kind, Token *cur, char *str)
{
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
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

        if (*p == '+' || *p == '-') {
            cur = new_token(TK_RESERVED, cur, p++);
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error_at(p, "トークナイズできません");
    }

    new_token(TK_EOF, cur, p);
    return head.next;
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "引数の個数が正しくありません。\n");
        return 1;
    }

    user_input = argv[1];

    token = tokenize(user_input);

    printf(".text\n");
    printf(".global main\n");
    printf("main:\n");
    printf("  mov x0, #%d\n", expect_number());

    while (!at_eof()) {
        if (consume('+')) {
            printf("  add x0, x0, #%d\n", expect_number());
            continue;
        }

        expect('-');
        printf("  sub x0, x0, #%d\n", expect_number());
    }

    printf("  mov x8, #93\n");
    printf("  svc #0\n");
    return 0;
}