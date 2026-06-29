/* ============================================================
 * main.c - Punto de entrada del traductor ConsultaLang
 * ------------------------------------------------------------
 * Flujo:
 *   1. Lee el codigo ConsultaLang (archivo argumento o stdin).
 *   2. Ejecuta el analisis lexico + sintactico (yyparse).
 *   3. Ejecuta el analisis semantico.
 *   4. Genera el SQL.
 *   5. Imprime un JSON con el resultado para el backend Flask.
 *   6. Guarda el SQL en output/salida.sql.
 * ============================================================ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "errores.h"
#include "semantica.h"
#include "generador.h"

extern int yyparse(void);
extern FILE *yyin;

/* Convierte el estado numerico de fase a texto */
static const char *estado_txt(int e) {
    if (e == 1)  return "correcto";
    if (e == 0)  return "error";
    return "no ejecutado";
}

/* Imprime una cadena escapada como string JSON */
static void json_str(FILE *out, const char *s) {
    fputc('"', out);
    for (const char *p = s; *p; p++) {
        switch (*p) {
            case '"':  fputs("\\\"", out); break;
            case '\\': fputs("\\\\", out); break;
            case '\n': fputs("\\n", out);  break;
            case '\r': fputs("\\r", out);  break;
            case '\t': fputs("\\t", out);  break;
            default:   fputc(*p, out);     break;
        }
    }
    fputc('"', out);
}

/* Imprime un arreglo JSON de mensajes */
static void json_array(FILE *out, char msgs[][MAX_LEN], int n) {
    fputc('[', out);
    for (int i = 0; i < n; i++) {
        if (i) fputc(',', out);
        json_str(out, msgs[i]);
    }
    fputc(']', out);
}

/* Guarda el SQL generado en output/salida.sql */
static void guardar_sql(const char *sql) {
    FILE *f = fopen("output/salida.sql", "w");
    if (f) {
        fputs(sql, f);
        fclose(f);
    }
}

int main(int argc, char **argv) {
    reporte_init();

    /* Entrada: archivo si se pasa argumento, si no stdin */
    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        if (!yyin) {
            fprintf(stderr, "No se pudo abrir el archivo: %s\n", argv[1]);
            return 1;
        }
    } else {
        yyin = stdin;
    }

    int pr = yyparse();

    char *sql = NULL;

    if (reporte.lexico == 0) {
        /* Error lexico: las fases siguientes no se ejecutan */
        reporte.sintactico = -1;
        reporte.semantico  = -1;
        reporte.traduccion = -1;
    } else if (pr != 0 || reporte.sintactico == 0) {
        /* Error sintactico */
        reporte.sintactico = 0;
        reporte.semantico  = -1;
        reporte.traduccion = -1;
    } else {
        /* Lexico + sintactico correctos: analisis semantico */
        reporte.sintactico = 1;
        int sem = analizar_semantica(raiz);
        if (sem == 0) {
            reporte.semantico = 1;
            sql = generar_sql(raiz);
            reporte.traduccion = 1;
            guardar_sql(sql);
        } else {
            reporte.semantico = 0;
            reporte.traduccion = -1;
        }
    }

    int success = (reporte.lexico == 1 && reporte.sintactico == 1 &&
                   reporte.semantico == 1 && reporte.traduccion == 1);

    /* ---------- Salida JSON ---------- */
    FILE *out = stdout;
    fputs("{", out);
    fprintf(out, "\"success\":%s,", success ? "true" : "false");

    fputs("\"sql\":", out);
    json_str(out, sql ? sql : "");
    fputs(",", out);

    fprintf(out, "\"lexico\":\"%s\",",     estado_txt(reporte.lexico));
    fprintf(out, "\"sintactico\":\"%s\",", estado_txt(reporte.sintactico));
    fprintf(out, "\"semantico\":\"%s\",",  estado_txt(reporte.semantico));
    fprintf(out, "\"traduccion\":\"%s\",", estado_txt(reporte.traduccion));

    fputs("\"errores\":", out);
    json_array(out, reporte.errores, reporte.nerrores);
    fputs(",", out);

    fputs("\"advertencias\":", out);
    json_array(out, reporte.advertencias, reporte.nadvertencias);

    fputs("}\n", out);

    if (sql) free(sql);
    if (yyin && yyin != stdin) fclose(yyin);
    return success ? 0 : 0;   /* siempre 0: el backend lee el JSON */
}
