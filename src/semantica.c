/* ============================================================
 * semantica.c - Validaciones semanticas de ConsultaLang
 * ------------------------------------------------------------
 * Reglas implementadas (segun la especificacion):
 *   SELECT  : tabla obligatoria, columnas no vacias.
 *   INSERT  : tabla y valores obligatorios, col=valor completos.
 *   UPDATE  : tabla y campos obligatorios.
 *   DELETE  : tabla obligatoria, advertencia si no hay WHERE.
 *   CREATE  : tabla, columnas, tipos validos, una sola PK,
 *             sin columnas repetidas.
 * ============================================================ */
#include <stdio.h>
#include <string.h>
#include "semantica.h"
#include "errores.h"

/* Cuenta cuantas columnas tiene una lista de SELECT */
static int contar_columnas(ColItem *c) {
    int n = 0;
    while (c) { n++; c = c->next; }
    return n;
}

/* Verifica que la tabla exista y no este vacia */
static int validar_tabla(const char *tabla) {
    if (!tabla || tabla[0] == '\0') {
        agregar_error("Error semantico: la tabla no fue especificada.");
        return 0;
    }
    return 1;
}

/* ---------- Validacion de alias y columnas calificadas (JOIN) ---------- */

#define MAX_ALIAS 32

/* Registra un calificador (alias o nombre de tabla) evitando duplicados de alias.
   Devuelve 0 si el alias ya existia (error). */
static int registrar_alias(char lista[][128], int *n, const char *nombre, int *dup) {
    for (int i = 0; i < *n; i++)
        if (strcmp(lista[i], nombre) == 0) { *dup = 1; return 0; }
    if (*n < MAX_ALIAS) { snprintf(lista[*n], 128, "%s", nombre); (*n)++; }
    return 1;
}

/* ¿el calificador esta declarado? */
static int alias_declarado(char lista[][128], int n, const char *pref) {
    for (int i = 0; i < n; i++)
        if (strcmp(lista[i], pref) == 0) return 1;
    return 0;
}

/* Valida el prefijo de una columna calificada "alias.columna" */
static int validar_col_calificada(const char *name, char lista[][128], int n) {
    const char *punto = strchr(name, '.');
    if (!punto) return 1;                 /* sin calificar: no se valida aqui */
    char pref[128];
    size_t l = (size_t)(punto - name);
    if (l >= sizeof(pref)) l = sizeof(pref) - 1;
    strncpy(pref, name, l); pref[l] = '\0';
    if (!alias_declarado(lista, n, pref)) {
        agregar_error("Error semantico: el alias o tabla '%s' no fue declarado (columna '%s').",
                      pref, name);
        return 0;
    }
    return 1;
}

/* Recorre condiciones validando columnas calificadas */
static int validar_cond_alias(Cond *c, char lista[][128], int n) {
    if (!c) return 1;
    int ok = 1;
    switch (c->type) {
        case C_AND: case C_OR:
            if (!validar_cond_alias(c->left, lista, n)) ok = 0;
            if (!validar_cond_alias(c->right, lista, n)) ok = 0;
            break;
        default:
            if (c->col && !validar_col_calificada(c->col, lista, n)) ok = 0;
            break;
    }
    return ok;
}

static int sem_select(Stmt *s) {
    int ok = 1;
    if (!validar_tabla(s->table)) ok = 0;
    if (!s->select_all && contar_columnas(s->columns) == 0) {
        agregar_error("Error semantico: la lista de columnas no puede estar vacia.");
        ok = 0;
    }

    /* Recolecta alias y nombres de tabla declarados */
    char lista[MAX_ALIAS][128];
    int n = 0, dup = 0;
    /* nombre de tabla principal siempre es un calificador valido */
    registrar_alias(lista, &n, s->table, &dup);
    if (s->alias) {
        dup = 0;
        registrar_alias(lista, &n, s->alias, &dup);
    }
    for (Join *j = s->joins; j; j = j->next) {
        registrar_alias(lista, &n, j->table, &dup);
        if (j->alias) {
            dup = 0;
            if (!registrar_alias(lista, &n, j->alias, &dup) || dup) {
                agregar_error("Error semantico: alias repetido '%s' en la consulta.", j->alias);
                ok = 0;
            }
        }
    }

    /* Solo validamos calificadores si hay JOIN o alias (consultas con prefijos) */
    if (s->joins || s->alias) {
        if (!s->select_all)
            for (ColItem *c = s->columns; c; c = c->next)
                if (!validar_col_calificada(c->name, lista, n)) ok = 0;
        if (!validar_cond_alias(s->where, lista, n)) ok = 0;
        for (Join *j = s->joins; j; j = j->next)
            if (!validar_cond_alias(j->on, lista, n)) ok = 0;
    }
    return ok;
}

static int sem_insert(Stmt *s) {
    int ok = 1;
    if (!validar_tabla(s->table)) ok = 0;
    if (s->assigns == NULL) {
        agregar_error("Error semantico: INSERT sin valores.");
        ok = 0;
    }
    for (Assign *a = s->assigns; a; a = a->next) {
        if (!a->col || a->col[0] == '\0' || a->val == NULL) {
            agregar_error("Error semantico: cada asignacion debe tener columna y valor.");
            ok = 0;
        }
    }
    return ok;
}

static int sem_update(Stmt *s) {
    int ok = 1;
    if (!validar_tabla(s->table)) ok = 0;
    if (s->assigns == NULL) {
        agregar_error("Error semantico: UPDATE sin campos a modificar.");
        ok = 0;
    }
    for (Assign *a = s->assigns; a; a = a->next) {
        if (!a->col || a->col[0] == '\0' || a->val == NULL) {
            agregar_error("Error semantico: cada asignacion debe tener columna y valor.");
            ok = 0;
        }
    }
    return ok;
}

static int sem_delete(Stmt *s) {
    int ok = 1;
    if (!validar_tabla(s->table)) ok = 0;
    if (s->where == NULL) {
        agregar_advertencia("Advertencia semantica: DELETE sin WHERE puede eliminar todos los registros.");
    }
    return ok;
}

static int sem_alter(Stmt *s) {
    int ok = 1;
    if (!validar_tabla(s->table)) ok = 0;
    if (s->coldefs == NULL) {
        agregar_error("Error semantico: ALTER TABLE sin columna a agregar.");
        ok = 0;
    }
    return ok;
}

static int sem_drop(Stmt *s) {
    return validar_tabla(s->table) ? 1 : 0;
}

static int sem_create(Stmt *s) {
    int ok = 1;
    if (!validar_tabla(s->table)) ok = 0;
    if (s->coldefs == NULL) {
        agregar_error("Error semantico: CREATE TABLE sin columnas.");
        return 0;
    }

    int n_primary = 0;
    /* Recorrido para PK multiple y columnas repetidas */
    for (ColDef *c = s->coldefs; c; c = c->next) {
        if (c->primary) n_primary++;

        /* Buscar duplicados mas adelante en la lista */
        for (ColDef *d = c->next; d; d = d->next) {
            if (strcmp(c->name, d->name) == 0) {
                agregar_error("Error semantico: columna repetida '%s' en la tabla '%s'.",
                              c->name, s->table);
                ok = 0;
            }
        }
    }

    if (n_primary > 1) {
        agregar_error("Error semantico: la tabla '%s' tiene mas de una clave primaria.",
                      s->table);
        ok = 0;
    }
    return ok;
}

int analizar_semantica(Stmt *s) {
    if (s == NULL) {
        agregar_error("Error semantico: no se construyo ninguna instruccion.");
        return 1;
    }

    int ok = 1;
    switch (s->type) {
        case ST_SELECT: ok = sem_select(s); break;
        case ST_INSERT: ok = sem_insert(s); break;
        case ST_UPDATE: ok = sem_update(s); break;
        case ST_DELETE: ok = sem_delete(s); break;
        case ST_CREATE: ok = sem_create(s); break;
        case ST_ALTER:  ok = sem_alter(s);  break;
        case ST_DROP:   ok = sem_drop(s);   break;
    }
    return ok ? 0 : 1;
}
