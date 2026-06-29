/* ============================================================
 * errores.h - Recoleccion de errores y advertencias
 * ------------------------------------------------------------
 * Modulo compartido por lexer, parser, semantica y main.
 * Guarda mensajes en listas para luego volcarlos en el JSON.
 * ============================================================ */
#ifndef ERRORES_H
#define ERRORES_H

#define MAX_MSGS 64
#define MAX_LEN  512

/* Estado de cada fase: 1=correcto, 0=error, -1=no ejecutado */
typedef struct {
    int lexico;
    int sintactico;
    int semantico;
    int traduccion;

    char errores[MAX_MSGS][MAX_LEN];
    int  nerrores;

    char advertencias[MAX_MSGS][MAX_LEN];
    int  nadvertencias;
} Reporte;

extern Reporte reporte;

void reporte_init(void);
void agregar_error(const char *fmt, ...);
void agregar_advertencia(const char *fmt, ...);

#endif /* ERRORES_H */
