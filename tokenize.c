#include "64cc.h"

char *user_input;
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
