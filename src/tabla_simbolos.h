/* ============================================================
 * tabla_simbolos.h - Tabla de simbolos dinamica de ConsultaLang
 * ------------------------------------------------------------
 * Recorre el AST de la instruccion actual y registra SOLO los
 * simbolos usados (tablas, alias, columnas, literales, funciones)
 * con sus columnas: Simbolo, Categoria, Tipo/Clase, Ambito, Info.
 * Se regenera en cada analisis (no es una lista fija).
 * ============================================================ */
#ifndef TABLA_SIMBOLOS_H
#define TABLA_SIMBOLOS_H

#include "ast.h"

void construir_tabla_simbolos(Stmt *s);

#endif /* TABLA_SIMBOLOS_H */
