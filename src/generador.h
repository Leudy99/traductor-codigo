/* ============================================================
 * generador.h - Generador de SQL de ConsultaLang
 * ------------------------------------------------------------
 * Recorre el AST ya validado y produce la cadena SQL estandar.
 * Devuelve un buffer reservado con malloc (el llamador lo libera).
 * ============================================================ */
#ifndef GENERADOR_H
#define GENERADOR_H

#include "ast.h"

/* Genera SQL a partir del AST. Devuelve cadena dinamica. */
char *generar_sql(Stmt *s);

#endif /* GENERADOR_H */
