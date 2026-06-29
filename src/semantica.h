/* ============================================================
 * semantica.h - Analisis semantico de ConsultaLang
 * ------------------------------------------------------------
 * Valida el AST antes de generar SQL. Devuelve 0 si todo es
 * correcto (puede haber advertencias) o distinto de 0 si hay
 * un error semantico que impide la traduccion.
 * ============================================================ */
#ifndef SEMANTICA_H
#define SEMANTICA_H

#include "ast.h"

/* Analiza la instruccion. 0 = ok, !=0 = error semantico. */
int analizar_semantica(Stmt *s);

#endif /* SEMANTICA_H */
