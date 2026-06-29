/* ============================================================
 * ast.c - Constructores del AST de ConsultaLang
 * Funciones simples que reservan memoria y rellenan nodos.
 * Las listas se construyen agregando al final para conservar
 * el orden en que aparecen en la entrada.
 * ============================================================ */
#include <stdlib.h>
#include <string.h>
#include "ast.h"

Stmt *raiz = NULL;

/* Copia portable de cadena (evita depender de strdup) */
char *cl_strdup(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s) + 1;
    char *p = (char *)malloc(n);
    if (p) memcpy(p, s, n);
    return p;
}

Stmt *nuevoStmt(StmtType type) {
    Stmt *s = (Stmt *)calloc(1, sizeof(Stmt));
    s->type = type;
    s->limit = -1;       /* sin limite por defecto */
    return s;
}

Value *nuevoValue(ValueType type, const char *text) {
    Value *v = (Value *)calloc(1, sizeof(Value));
    v->type = type;
    v->text = cl_strdup(text);
    return v;
}

ColItem *nuevaColumna(const char *name) {
    ColItem *c = (ColItem *)calloc(1, sizeof(ColItem));
    c->name = cl_strdup(name);
    return c;
}

/* Agrega 'nueva' al final de 'lista' y devuelve la cabeza */
ColItem *agregarColumna(ColItem *lista, ColItem *nueva) {
    if (!lista) return nueva;
    ColItem *p = lista;
    while (p->next) p = p->next;
    p->next = nueva;
    return lista;
}

Cond *nuevaComparacion(const char *col, const char *op, Value *val) {
    Cond *c = (Cond *)calloc(1, sizeof(Cond));
    c->type = C_CMP;
    c->col = cl_strdup(col);
    c->op = cl_strdup(op);
    c->val = val;
    return c;
}

Cond *nuevaLogica(CondType type, Cond *left, Cond *right) {
    Cond *c = (Cond *)calloc(1, sizeof(Cond));
    c->type = type;
    c->left = left;
    c->right = right;
    return c;
}

Assign *nuevaAsignacion(const char *col, Value *val) {
    Assign *a = (Assign *)calloc(1, sizeof(Assign));
    a->col = cl_strdup(col);
    a->val = val;
    return a;
}

Assign *agregarAsignacion(Assign *lista, Assign *nueva) {
    if (!lista) return nueva;
    Assign *p = lista;
    while (p->next) p = p->next;
    p->next = nueva;
    return lista;
}

OrderBy *nuevoOrden(const char *col, const char *dir) {
    OrderBy *o = (OrderBy *)calloc(1, sizeof(OrderBy));
    o->col = cl_strdup(col);
    o->dir = cl_strdup(dir);
    return o;
}

ColDef *nuevaColDef(const char *name, const char *type) {
    ColDef *c = (ColDef *)calloc(1, sizeof(ColDef));
    c->name = cl_strdup(name);
    c->type = cl_strdup(type);
    return c;
}

ColDef *agregarColDef(ColDef *lista, ColDef *nueva) {
    if (!lista) return nueva;
    ColDef *p = lista;
    while (p->next) p = p->next;
    p->next = nueva;
    return lista;
}
