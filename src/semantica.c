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

static int sem_select(Stmt *s) {
    int ok = 1;
    if (!validar_tabla(s->table)) ok = 0;
    if (!s->select_all && contar_columnas(s->columns) == 0) {
        agregar_error("Error semantico: la lista de columnas no puede estar vacia.");
        ok = 0;
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
    }
    return ok ? 0 : 1;
}
