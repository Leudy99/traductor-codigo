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

    /* Fase de optimizacion */
    char optimizaciones[MAX_MSGS][MAX_LEN];
    int  noptimizaciones;

    char adv_optimizador[MAX_MSGS][MAX_LEN];
    int  nadv_optimizador;

    /* Generador de codigo destino */
    char adv_destino[MAX_MSGS][MAX_LEN];
    int  nadv_destino;

    /* Tabla de tokens (analisis lexico) */
    char tok_tipo[MAX_MSGS][32];
    char tok_lex[MAX_MSGS][128];
    int  tok_lin[MAX_MSGS];
    int  tok_col[MAX_MSGS];
    int  ntokens;

    /* Tabla de simbolos (5 columnas, dinamica) */
    char s_nombre[MAX_MSGS][64];
    char s_categoria[MAX_MSGS][24];
    char s_tipo[MAX_MSGS][24];
    char s_ambito[MAX_MSGS][48];
    char s_info[MAX_MSGS][96];
    int  nsim;
} Reporte;

extern Reporte reporte;

void reporte_init(void);
void agregar_error(const char *fmt, ...);
void agregar_advertencia(const char *fmt, ...);
void agregar_optimizacion(const char *fmt, ...);
void agregar_adv_optimizador(const char *fmt, ...);
void agregar_adv_destino(const char *fmt, ...);
void agregar_token(const char *tipo, const char *lexema, int linea, int columna);

#endif /* ERRORES_H */
