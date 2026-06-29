/* ============================================================
 * errores.c - Implementacion del recolector de errores
 * ============================================================ */
#include <stdio.h>
#include <stdarg.h>
#include "errores.h"

Reporte reporte;

void reporte_init(void) {
    reporte.lexico = 1;        /* asumimos correcto hasta que falle */
    reporte.sintactico = 1;
    reporte.semantico = -1;    /* aun no ejecutado */
    reporte.traduccion = -1;
    reporte.nerrores = 0;
    reporte.nadvertencias = 0;
}

void agregar_error(const char *fmt, ...) {
    if (reporte.nerrores >= MAX_MSGS) return;
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(reporte.errores[reporte.nerrores], MAX_LEN, fmt, ap);
    va_end(ap);
    reporte.nerrores++;
}

void agregar_advertencia(const char *fmt, ...) {
    if (reporte.nadvertencias >= MAX_MSGS) return;
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(reporte.advertencias[reporte.nadvertencias], MAX_LEN, fmt, ap);
    va_end(ap);
    reporte.nadvertencias++;
}
