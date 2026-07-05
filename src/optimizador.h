/* ============================================================
 * optimizador.h - Fase de optimizacion de codigo de ConsultaLang
 * ------------------------------------------------------------
 * Recibe el SQL bruto (ya generado) y produce una version
 * optimizada aplicando reglas simples sobre la clausula WHERE.
 * Registra las optimizaciones y advertencias en el 'reporte'.
 * ============================================================ */
#ifndef OPTIMIZADOR_H
#define OPTIMIZADOR_H

/* Optimiza el SQL. Devuelve cadena dinamica (el llamador la libera). */
char *optimizar_sql(const char *sql_original);

#endif /* OPTIMIZADOR_H */
