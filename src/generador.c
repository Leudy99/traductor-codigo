/* ============================================================
 * generador.c - Traduccion de AST a SQL estandar
 * ------------------------------------------------------------
 * Cada tipo de instruccion tiene su funcion. Se usa un buffer
 * de texto que crece por concatenacion (append) para armar la
 * salida con saltos de linea como en SQL formateado.
 * ============================================================ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "generador.h"

#define SQL_MAX 8192

/* Concatena 'txt' al final de 'buf' respetando el tamano maximo */
static void append(char *buf, const char *txt) {
    size_t len = strlen(buf);
    size_t libre = SQL_MAX - len - 1;
    strncat(buf, txt, libre);
}

/* Convierte un Value a su forma SQL */
static void valor_sql(Value *v, char *out, size_t n) {
    if (!v) { out[0] = '\0'; return; }
    switch (v->type) {
        case V_STRING:
            /* Texto entre comillas simples en SQL */
            snprintf(out, n, "'%s'", v->text);
            break;
        case V_BOOL:    /* TRUE / FALSE ya normalizado */
        case V_INT:
        case V_DECIMAL:
        case V_IDENT:
        default:
            snprintf(out, n, "%s", v->text);
            break;
    }
}

/* Traduce el arbol de condiciones (WHERE / HAVING / ON) */
static void condicion_sql(Cond *c, char *buf) {
    if (!c) return;
    char tmp[1400], vsql[600], vsql2[600];
    switch (c->type) {
        case C_CMP:
            valor_sql(c->val, vsql, sizeof(vsql));
            snprintf(tmp, sizeof(tmp), "%s %s %s", c->col, c->op, vsql);
            append(buf, tmp);
            break;
        case C_AND:
            condicion_sql(c->left, buf);
            append(buf, " AND ");
            condicion_sql(c->right, buf);
            break;
        case C_OR:
            condicion_sql(c->left, buf);
            append(buf, " OR ");
            condicion_sql(c->right, buf);
            break;
        case C_BETWEEN:
            valor_sql(c->val, vsql, sizeof(vsql));
            valor_sql(c->val2, vsql2, sizeof(vsql2));
            snprintf(tmp, sizeof(tmp), "%s BETWEEN %s AND %s", c->col, vsql, vsql2);
            append(buf, tmp);
            break;
        case C_IN:
            snprintf(tmp, sizeof(tmp), "%s IN (", c->col);
            append(buf, tmp);
            for (ValList *l = c->inlist; l; l = l->next) {
                valor_sql(l->val, vsql, sizeof(vsql));
                append(buf, vsql);
                if (l->next) append(buf, ", ");
            }
            append(buf, ")");
            break;
        case C_LIKE:
            valor_sql(c->val, vsql, sizeof(vsql));
            snprintf(tmp, sizeof(tmp), "%s LIKE %s", c->col, vsql);
            append(buf, tmp);
            break;
        case C_ISNULL:
            snprintf(tmp, sizeof(tmp), "%s IS NULL", c->col);
            append(buf, tmp);
            break;
        case C_ISNOTNULL:
            snprintf(tmp, sizeof(tmp), "%s IS NOT NULL", c->col);
            append(buf, tmp);
            break;
    }
}

/* ---------- SELECT ---------- */
static void gen_select(Stmt *s, char *buf) {
    append(buf, "SELECT ");
    if (s->distinct) append(buf, "DISTINCT ");
    if (s->select_all) {
        append(buf, "*");
    } else {
        for (ColItem *c = s->columns; c; c = c->next) {
            append(buf, c->name);
            if (c->next) append(buf, ", ");
        }
    }

    /* FROM tabla [alias] */
    append(buf, "\nFROM ");
    append(buf, s->table);
    if (s->alias) { append(buf, " "); append(buf, s->alias); }

    /* JOINs */
    for (Join *j = s->joins; j; j = j->next) {
        append(buf, "\n");
        append(buf, j->tipo);
        append(buf, " ");
        append(buf, j->table);
        if (j->alias) { append(buf, " "); append(buf, j->alias); }
        append(buf, " ON ");
        condicion_sql(j->on, buf);
    }

    if (s->where) {
        append(buf, "\nWHERE ");
        condicion_sql(s->where, buf);
    }
    if (s->group_by) {
        append(buf, "\nGROUP BY ");
        for (ColItem *c = s->group_by; c; c = c->next) {
            append(buf, c->name);
            if (c->next) append(buf, ", ");
        }
    }
    if (s->having) {
        append(buf, "\nHAVING ");
        condicion_sql(s->having, buf);
    }
    if (s->order) {
        char tmp[300];
        snprintf(tmp, sizeof(tmp), "\nORDER BY %s %s", s->order->col, s->order->dir);
        append(buf, tmp);
    }
    if (s->limit >= 0) {
        char tmp[64];
        snprintf(tmp, sizeof(tmp), "\nLIMIT %d", s->limit);
        append(buf, tmp);
    }
    append(buf, ";");
}

/* ---------- ALTER TABLE ---------- */
static void gen_alter(Stmt *s, char *buf) {
    /* Solo se soporta ADD columna */
    ColDef *c = s->coldefs;
    char tmp[600];
    if (c) {
        snprintf(tmp, sizeof(tmp), "ALTER TABLE %s ADD %s %s;", s->table, c->name, c->type);
    } else {
        snprintf(tmp, sizeof(tmp), "ALTER TABLE %s;", s->table);
    }
    append(buf, tmp);
}

/* ---------- DROP TABLE ---------- */
static void gen_drop(Stmt *s, char *buf) {
    char tmp[600];
    snprintf(tmp, sizeof(tmp), "DROP TABLE %s;", s->table);
    append(buf, tmp);
}

/* ---------- INSERT ---------- */
static void gen_insert(Stmt *s, char *buf) {
    char tmp[1024];
    snprintf(tmp, sizeof(tmp), "INSERT INTO %s (", s->table);
    append(buf, tmp);

    for (Assign *a = s->assigns; a; a = a->next) {
        append(buf, a->col);
        if (a->next) append(buf, ", ");
    }
    append(buf, ")\nVALUES (");

    for (Assign *a = s->assigns; a; a = a->next) {
        char vsql[600];
        valor_sql(a->val, vsql, sizeof(vsql));
        append(buf, vsql);
        if (a->next) append(buf, ", ");
    }
    append(buf, ");");
}

/* ---------- UPDATE ---------- */
static void gen_update(Stmt *s, char *buf) {
    char tmp[600];
    snprintf(tmp, sizeof(tmp), "UPDATE %s\nSET ", s->table);
    append(buf, tmp);

    for (Assign *a = s->assigns; a; a = a->next) {
        char vsql[600], par[1300];
        valor_sql(a->val, vsql, sizeof(vsql));
        snprintf(par, sizeof(par), "%s = %s", a->col, vsql);
        append(buf, par);
        if (a->next) append(buf, ", ");
    }
    if (s->where) {
        append(buf, "\nWHERE ");
        condicion_sql(s->where, buf);
    }
    append(buf, ";");
}

/* ---------- DELETE ---------- */
static void gen_delete(Stmt *s, char *buf) {
    char tmp[600];
    snprintf(tmp, sizeof(tmp), "DELETE FROM %s", s->table);
    append(buf, tmp);
    if (s->where) {
        append(buf, "\nWHERE ");
        condicion_sql(s->where, buf);
    }
    append(buf, ";");
}

/* ---------- CREATE TABLE ---------- */
static void gen_create(Stmt *s, char *buf) {
    char tmp[600];
    snprintf(tmp, sizeof(tmp), "CREATE TABLE %s (\n", s->table);
    append(buf, tmp);

    for (ColDef *c = s->coldefs; c; c = c->next) {
        char linea[700];
        snprintf(linea, sizeof(linea), "    %s %s", c->name, c->type);
        append(buf, linea);
        if (c->primary) append(buf, " PRIMARY KEY");
        if (c->notnull) append(buf, " NOT NULL");
        if (c->unique)  append(buf, " UNIQUE");
        if (c->next) append(buf, ",");
        append(buf, "\n");
    }
    append(buf, ");");
}

char *generar_sql(Stmt *s) {
    char *buf = (char *)calloc(SQL_MAX, 1);
    if (!s) return buf;

    switch (s->type) {
        case ST_SELECT: gen_select(s, buf); break;
        case ST_INSERT: gen_insert(s, buf); break;
        case ST_UPDATE: gen_update(s, buf); break;
        case ST_DELETE: gen_delete(s, buf); break;
        case ST_CREATE: gen_create(s, buf); break;
        case ST_ALTER:  gen_alter(s, buf);  break;
        case ST_DROP:   gen_drop(s, buf);   break;
    }
    return buf;
}
