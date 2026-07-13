/* ============================================================
 * generador_destino.c - Adaptacion al dialecto SQL destino
 * ------------------------------------------------------------
 * Transformaciones simples y educativas:
 *   - estandar / mysql / postgresql: conservan LIMIT.
 *   - sqlserver: convierte "LIMIT N" en "SELECT TOP N ...".
 * Registra advertencias en el reporte cuando aplica.
 * ============================================================ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "generador_destino.h"
#include "errores.h"

#define BUF 8192

/* Pasa a minusculas una copia corta */
static void a_minusculas(const char *in, char *out, size_t n) {
    size_t i = 0;
    for (; in[i] && i < n - 1; i++) out[i] = (char)tolower((unsigned char)in[i]);
    out[i] = '\0';
}

const char *dialecto_nombre(const char *dialecto) {
    char d[32];
    a_minusculas(dialecto ? dialecto : "", d, sizeof(d));
    if (strcmp(d, "mysql") == 0)       return "MySQL";
    if (strcmp(d, "postgresql") == 0)  return "PostgreSQL";
    if (strcmp(d, "postgres") == 0)    return "PostgreSQL";
    if (strcmp(d, "sqlserver") == 0)   return "SQL Server";
    if (strcmp(d, "sql server") == 0)  return "SQL Server";
    return "SQL estandar";
}

/* Convierte LIMIT N a TOP N (SQL Server) sobre un SELECT.
   Devuelve 1 si hubo conversion. */
static int limit_a_top(const char *sql, char *out, size_t n) {
    /* Busca "\nLIMIT " o " LIMIT " */
    const char *lp = strstr(sql, "LIMIT ");
    const char *sel = strstr(sql, "SELECT ");
    if (!lp || !sel) { snprintf(out, n, "%s", sql); return 0; }

    /* Lee el numero tras LIMIT */
    const char *num = lp + 6;
    while (*num == ' ') num++;
    int valor = atoi(num);
    if (valor <= 0) { snprintf(out, n, "%s", sql); return 0; }

    /* Punto donde empieza la clausula LIMIT (incluye el salto/espacio previo) */
    const char *ini = lp;
    while (ini > sql && (ini[-1] == '\n' || ini[-1] == ' ')) ini--;
    /* Fin de la clausula LIMIT: hasta ';' o fin */
    const char *fin = lp + 6;
    while (*fin && *fin != ';' && *fin != '\n') fin++;

    /* Punto de insercion de TOP: justo despues de "SELECT " y de "DISTINCT " si existe */
    const char *ins = sel + 7;                 /* tras "SELECT " */
    if (strncmp(ins, "DISTINCT ", 9) == 0) ins += 9;

    /* Reconstruye: [inicio..ins) + "TOP N " + [ins..ini) + [fin..] */
    size_t o = 0;
    /* parte 1: desde el inicio hasta el punto de insercion */
    for (const char *p = sql; p < ins && o < n - 1; p++) out[o++] = *p;
    /* TOP N */
    o += snprintf(out + o, n - o, "TOP %d ", valor);
    /* parte 2: desde ins hasta el inicio del LIMIT */
    for (const char *p = ins; p < ini && o < n - 1; p++) out[o++] = *p;
    /* parte 3: desde el fin del LIMIT hasta el final */
    for (const char *p = fin; *p && o < n - 1; p++) out[o++] = *p;
    out[o] = '\0';
    return 1;
}

char *generar_codigo_destino(const char *sql, const char *dialecto) {
    char *res = (char *)calloc(BUF, 1);
    if (!sql) return res;

    char d[32];
    a_minusculas(dialecto ? dialecto : "", d, sizeof(d));

    int es_sqlserver = (strcmp(d, "sqlserver") == 0 || strcmp(d, "sql server") == 0);

    if (es_sqlserver) {
        if (limit_a_top(sql, res, BUF)) {
            agregar_adv_destino("SQL Server: 'LIMIT N' se convirtio a 'SELECT TOP N'.");
        }
    } else {
        snprintf(res, BUF, "%s", sql);
        /* mysql / postgresql / estandar conservan LIMIT tal cual */
    }
    return res;
}
