/* ============================================================
 * tabla_simbolos.c - Construccion dinamica de la tabla de simbolos
 * ------------------------------------------------------------
 * Genera filas {Simbolo, Categoria, Tipo/Clase, Ambito, Info}
 * a partir del AST. Evita duplicados y fusiona la informacion
 * cuando un mismo simbolo aparece en mas de un contexto.
 * ============================================================ */
#include <stdio.h>
#include <string.h>
#include "tabla_simbolos.h"
#include "errores.h"

/* Registra (o fusiona) un simbolo en la tabla del reporte */
static void reg(const char *nombre, const char *cat, const char *tipo,
                const char *ambito, const char *info) {
    if (!nombre || !nombre[0]) return;
    /* Buscar existente con mismo nombre + ambito + categoria */
    for (int i = 0; i < reporte.nsim; i++) {
        if (strcmp(reporte.s_nombre[i], nombre) == 0 &&
            strcmp(reporte.s_ambito[i], ambito) == 0 &&
            strcmp(reporte.s_categoria[i], cat) == 0) {
            /* Fusionar la info si es un contexto nuevo */
            if (info && info[0] && !strstr(reporte.s_info[i], info)) {
                size_t l = strlen(reporte.s_info[i]);
                snprintf(reporte.s_info[i] + l, 96 - l, " / %s", info);
            }
            return;
        }
    }
    if (reporte.nsim >= 64) return;
    int i = reporte.nsim;
    snprintf(reporte.s_nombre[i], 64, "%s", nombre);
    snprintf(reporte.s_categoria[i], 24, "%s", cat);
    snprintf(reporte.s_tipo[i], 24, "%s", tipo);
    snprintf(reporte.s_ambito[i], 48, "%s", ambito);
    snprintf(reporte.s_info[i], 96, "%s", info ? info : "");
    reporte.nsim++;
}

/* Separa "alias.columna" -> columna + ambito=alias. Si no hay punto,
   usa la tabla por defecto como ambito. */
static void separar(const char *name, char *col, char *amb, const char *tabladef) {
    const char *punto = strchr(name, '.');
    if (punto) {
        size_t l = (size_t)(punto - name);
        if (l >= 48) l = 47;
        strncpy(amb, name, l); amb[l] = '\0';
        snprintf(col, 64, "%s", punto + 1);
    } else {
        snprintf(col, 64, "%s", name);
        snprintf(amb, 48, "%s", tabladef && tabladef[0] ? tabladef : "global");
    }
}

/* Registra el lado izquierdo de una condicion (columna o funcion) */
static void reg_operando(const char *name, const char *ctx, const char *tabladef) {
    if (!name) return;
    if (strchr(name, '(')) {                 /* funcion de agregacion */
        reg(name, "funcion", "agregacion", "consulta", ctx);
        return;
    }
    char col[64], amb[48];
    separar(name, col, amb, tabladef);
    reg(col, "columna", "identificador", amb, ctx);
}

/* Registra un valor: literal o columna (si es identificador) */
static void reg_valor(Value *v, const char *ctx, const char *tabladef) {
    if (!v) return;
    if (v->type == V_IDENT) {
        char col[64], amb[48];
        separar(v->text, col, amb, tabladef);
        reg(col, "columna", "identificador", amb, ctx);
    } else {
        const char *tp = v->type == V_INT ? "entero"
                       : v->type == V_DECIMAL ? "decimal"
                       : v->type == V_STRING ? "cadena"
                       : v->type == V_BOOL ? "booleano" : "identificador";
        reg(v->text, "literal", tp, ctx, "valor");
    }
}

/* Recorre una condicion registrando columnas y literales */
static void sim_cond(Cond *c, const char *ctx, const char *tabladef) {
    if (!c) return;
    switch (c->type) {
        case C_AND: case C_OR:
            sim_cond(c->left, ctx, tabladef);
            sim_cond(c->right, ctx, tabladef);
            break;
        case C_CMP:
            reg_operando(c->col, ctx, tabladef);
            reg_valor(c->val, ctx, tabladef);
            break;
        case C_BETWEEN:
            reg_operando(c->col, ctx, tabladef);
            reg_valor(c->val, ctx, tabladef);
            reg_valor(c->val2, ctx, tabladef);
            break;
        case C_IN:
            reg_operando(c->col, ctx, tabladef);
            for (ValList *l = c->inlist; l; l = l->next)
                reg_valor(l->val, ctx, tabladef);
            break;
        case C_LIKE:
            reg_operando(c->col, ctx, tabladef);
            reg_valor(c->val, ctx, tabladef);
            break;
        case C_ISNULL: case C_ISNOTNULL:
            reg_operando(c->col, ctx, tabladef);
            break;
    }
}

static void sim_select(Stmt *s) {
    const char *td = s->table;

    /* Tabla principal + alias */
    reg(s->table, "tabla", "identificador", "global", "FROM");
    if (s->alias) reg(s->alias, "alias", "tabla", s->table, "alias");

    /* JOINs */
    for (Join *j = s->joins; j; j = j->next) {
        reg(j->table, "tabla", "identificador", "global", "JOIN");
        if (j->alias) reg(j->alias, "alias", "tabla", j->table, "alias");
        sim_cond(j->on, "ON", td);
    }

    /* Columnas del SELECT */
    if (!s->select_all) {
        for (ColItem *c = s->columns; c; c = c->next)
            reg_operando(c->name, "SELECT", td);
    }

    /* WHERE / GROUP BY / HAVING / ORDER BY */
    sim_cond(s->where, "WHERE", td);
    for (ColItem *c = s->group_by; c; c = c->next)
        reg_operando(c->name, "GROUP BY", td);
    sim_cond(s->having, "HAVING", td);
    if (s->order)
        reg_operando(s->order->col, "ORDER BY", td);
}

void construir_tabla_simbolos(Stmt *s) {
    if (!s) return;
    switch (s->type) {
        case ST_SELECT:
            sim_select(s);
            break;
        case ST_INSERT:
            reg(s->table, "tabla", "identificador", "global", "INSERT");
            for (Assign *a = s->assigns; a; a = a->next) {
                reg(a->col, "columna", "identificador", s->table, "INSERT");
                reg_valor(a->val, "VALUES", s->table);
            }
            break;
        case ST_UPDATE:
            reg(s->table, "tabla", "identificador", "global", "UPDATE");
            for (Assign *a = s->assigns; a; a = a->next) {
                reg(a->col, "columna", "identificador", s->table, "SET");
                reg_valor(a->val, "SET", s->table);
            }
            sim_cond(s->where, "WHERE", s->table);
            break;
        case ST_DELETE:
            reg(s->table, "tabla", "identificador", "global", "DELETE");
            sim_cond(s->where, "WHERE", s->table);
            break;
        case ST_CREATE:
            reg(s->table, "tabla", "identificador", "global", "CREATE");
            for (ColDef *c = s->coldefs; c; c = c->next)
                reg(c->name, "columna", c->type, s->table, "definicion");
            break;
        case ST_ALTER:
            reg(s->table, "tabla", "identificador", "global", "ALTER");
            if (s->coldefs)
                reg(s->coldefs->name, "columna", s->coldefs->type, s->table, "ADD");
            break;
        case ST_DROP:
            reg(s->table, "tabla", "identificador", "global", "DROP");
            break;
    }
}
