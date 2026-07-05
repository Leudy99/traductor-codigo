/* ============================================================
 * optimizador.c - Optimizacion de codigo SQL de ConsultaLang
 * ------------------------------------------------------------
 * Trabaja sobre el texto del SQL bruto, concentrandose en la
 * clausula WHERE. Reglas educativas y simples de explicar:
 *   1. Eliminar condiciones repetidas (AND).
 *   2. Simplificar condiciones OR repetidas.
 *   3. Convertir OR sobre el mismo campo en IN (...).
 *   4. Detectar contradicciones simples (rango imposible): advertencia.
 *   5. Normalizar espacios del SQL final.
 * ============================================================ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "optimizador.h"
#include "errores.h"

#define MAX_PRED 64
#define BUF      8192

/* ---------- utilidades de cadena ---------- */

/* Recorta espacios a los lados (in-place) */
static void trim(char *s) {
    size_t n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n - 1])) s[--n] = '\0';
    size_t i = 0;
    while (s[i] && isspace((unsigned char)s[i])) i++;
    if (i) memmove(s, s + i, strlen(s + i) + 1);
}

/* Colapsa espacios multiples a uno solo (conserva '\n') */
static void normaliza_espacios(char *s) {
    char out[BUF];
    size_t o = 0;
    int prev_space = 0;
    for (size_t i = 0; s[i] && o < BUF - 1; i++) {
        if (s[i] == ' ') {
            if (prev_space) continue;
            prev_space = 1;
            out[o++] = ' ';
        } else {
            /* elimina el espacio justo antes de un salto de linea */
            if (s[i] == '\n' && o > 0 && out[o - 1] == ' ') o--;
            prev_space = 0;
            out[o++] = s[i];
        }
    }
    out[o] = '\0';
    strcpy(s, out);
}

/* Quita espacios/saltos finales (in-place) */
static void rtrim(char *s) {
    size_t n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n - 1])) s[--n] = '\0';
}

/* Divide "campo op valor". El valor es el resto (admite comillas con
   espacios, p. ej. 'Santo Domingo'). Devuelve 1 si hay campo, op y valor. */
static int partes(const char *pred, char *campo, char *op, char *valor) {
    int off = 0;
    if (sscanf(pred, "%127s %15s %n", campo, op, &off) < 2) return 0;
    if (off == 0) return 0;                 /* no hay valor */
    snprintf(valor, 128, "%s", pred + off);
    trim(valor);
    return valor[0] != '\0';
}

/* ¿el token es un numero? */
static int es_numero(const char *v) {
    if (!*v) return 0;
    int punto = 0, i = 0;
    if (v[0] == '+' || v[0] == '-') i = 1;
    if (!v[i]) return 0;
    for (; v[i]; i++) {
        if (v[i] == '.') { if (punto) return 0; punto = 1; }
        else if (!isdigit((unsigned char)v[i])) return 0;
    }
    return 1;
}

/* ---------- deteccion de contradiccion (campo > A AND campo < B) ---------- */
static int es_contradiccion(char preds[][256], char conns[][8], int n) {
    for (int i = 0; i + 1 < n; i++) {
        /* el conector entre pred[i] y pred[i+1] se guarda en conns[i+1] */
        if (strcmp(conns[i + 1], "AND") != 0) continue;
        char c1[128], o1[16], v1[128], c2[128], o2[16], v2[128];
        if (!partes(preds[i], c1, o1, v1)) continue;
        if (!partes(preds[i + 1], c2, o2, v2)) continue;
        if (strcmp(c1, c2) != 0) continue;
        if (!es_numero(v1) || !es_numero(v2)) continue;

        double n1 = atof(v1), n2 = atof(v2);
        int mayor1 = (strcmp(o1, ">") == 0 || strcmp(o1, ">=") == 0);
        int menor1 = (strcmp(o1, "<") == 0 || strcmp(o1, "<=") == 0);
        int mayor2 = (strcmp(o2, ">") == 0 || strcmp(o2, ">=") == 0);
        int menor2 = (strcmp(o2, "<") == 0 || strcmp(o2, "<=") == 0);

        /* cota inferior (>) y cota superior (<) que no se cruzan */
        if (mayor1 && menor2 && n1 >= n2) return 1;
        if (menor1 && mayor2 && n2 >= n1) return 1;
    }
    return 0;
}

char *optimizar_sql(const char *sql_original) {
    char *res = (char *)calloc(BUF, 1);
    if (!sql_original) { return res; }

    /* Localiza la clausula WHERE */
    const char *wp = strstr(sql_original, "WHERE ");
    if (!wp) {
        /* Sin WHERE: solo normaliza espacios */
        strncpy(res, sql_original, BUF - 1);
        normaliza_espacios(res);
        return res;
    }

    size_t pre_len = (size_t)(wp - sql_original);
    char pre[BUF]; strncpy(pre, sql_original, pre_len); pre[pre_len] = '\0';

    const char *bstart = wp + 6;                 /* tras "WHERE " */
    const char *bend = bstart;
    while (*bend && *bend != '\n' && *bend != ';') bend++;

    char body[BUF];
    size_t blen = (size_t)(bend - bstart);
    strncpy(body, bstart, blen); body[blen] = '\0';

    const char *suf = bend;                       /* '\n...' o ';' o fin */

    /* ---------- separar el cuerpo en predicados y conectores ---------- */
    char preds[MAX_PRED][256];
    char conns[MAX_PRED][8];                       /* conector ANTES del predicado i (i>=1) */
    int n = 0;
    const char *rest = body;
    while (n < MAX_PRED) {
        const char *pa = strstr(rest, " AND ");
        const char *po = strstr(rest, " OR ");
        const char *cut; const char *conn; int adv;
        if (pa && (!po || pa < po)) { cut = pa; conn = "AND"; adv = 5; }
        else if (po)                { cut = po; conn = "OR";  adv = 4; }
        else {
            snprintf(preds[n], 256, "%s", rest); trim(preds[n]);
            if (n == 0) conns[0][0] = '\0';
            n++;
            break;
        }
        size_t l = (size_t)(cut - rest);
        char tmp[256]; strncpy(tmp, rest, l); tmp[l] = '\0'; trim(tmp);
        snprintf(preds[n], 256, "%s", tmp);
        if (n == 0) conns[0][0] = '\0';
        n++;
        snprintf(conns[n], 8, "%s", conn);   /* conector que precede al siguiente */
        rest = cut + adv;
    }

    /* ---------- Regla 5: contradiccion ---------- */
    /* Regla 5: solo advertencia, no modifica el SQL */
    if (es_contradiccion(preds, conns, n)) {
        agregar_adv_optimizador("Advertencia: la condicion parece contradictoria (rango imposible).");
    }

    /* ---------- Reglas 1 y 3: eliminar predicados repetidos ---------- */
    char kp[MAX_PRED][256];
    char kc[MAX_PRED][8];
    int k = 0, dup_and = 0, dup_or = 0;
    for (int i = 0; i < n; i++) {
        int repetido = 0;
        for (int j = 0; j < k; j++)
            if (strcmp(preds[i], kp[j]) == 0) { repetido = 1; break; }
        if (repetido) {
            if (i > 0 && strcmp(conns[i], "OR") == 0) dup_or = 1;
            else dup_and = 1;
            continue;
        }
        snprintf(kp[k], 256, "%s", preds[i]);
        if (k == 0) kc[0][0] = '\0';
        else        snprintf(kc[k], 8, "%s", (i > 0 ? conns[i] : "AND"));
        k++;
    }
    if (dup_and) agregar_optimizacion("Se elimino una condicion repetida en la clausula WHERE.");
    if (dup_or)  agregar_optimizacion("Se simplifico una condicion OR repetida.");

    /* ---------- Regla 4: OR sobre mismo campo -> IN (...) ---------- */
    if (k >= 2) {
        int todos_or = 1, todos_eq = 1;
        char campo0[128] = "", c[128], o[16], v[128];
        for (int i = 0; i < k; i++) {
            if (i > 0 && strcmp(kc[i], "OR") != 0) { todos_or = 0; break; }
            if (!partes(kp[i], c, o, v) || strcmp(o, "=") != 0) { todos_eq = 0; break; }
            if (i == 0) snprintf(campo0, 128, "%s", c);
            else if (strcmp(campo0, c) != 0) { todos_eq = 0; break; }
        }
        if (todos_or && todos_eq) {
            char in[BUF]; int off;
            off = snprintf(in, BUF, "%s IN (", campo0);
            for (int i = 0; i < k; i++) {
                partes(kp[i], c, o, v);
                off += snprintf(in + off, BUF - off, "%s%s", (i ? ", " : ""), v);
            }
            snprintf(in + off, BUF - off, ")");
            snprintf(kp[0], 256, "%s", in);
            kc[0][0] = '\0';
            k = 1;
            agregar_optimizacion("Se convirtieron condiciones OR sobre el mismo campo en una clausula IN.");
        }
    }

    /* pasa el resultado a preds/conns */
    for (int i = 0; i < k; i++) {
        snprintf(preds[i], 256, "%s", kp[i]);
        snprintf(conns[i], 8, "%s", kc[i]);
    }
    n = k;

    /* ---------- reconstruir el SQL ---------- */
    if (n == 0) {
        /* WHERE quedo vacio: se elimina la clausula */
        rtrim(pre);
        snprintf(res, BUF, "%s%s", pre, suf);
    } else {
        char nbody[BUF]; int off = 0;
        off += snprintf(nbody + off, BUF - off, "%s", preds[0]);
        for (int i = 1; i < n; i++)
            off += snprintf(nbody + off, BUF - off, " %s %s", conns[i], preds[i]);
        snprintf(res, BUF, "%sWHERE %s%s", pre, nbody, suf);
    }

    normaliza_espacios(res);
    return res;
}
