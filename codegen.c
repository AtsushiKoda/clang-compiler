#include "64cc.h"

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

void codegen(Node *node)
{
    printf(".text\n");
    printf(".global main\n");
    printf("main:\n");

    // 抽象構文木を下りながらコード生成
    gen(node);
    
    printf("  ldr x0, [sp], #16\n");
    printf("  mov x8, #93\n");
    printf("  svc #0\n");
}