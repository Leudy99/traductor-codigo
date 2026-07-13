/* ============================================================
 * generador_destino.h - Generador de codigo destino
 * ------------------------------------------------------------
 * Fase final: adapta el SQL optimizado al dialecto elegido.
 * Dialectos: estandar, mysql, postgresql, sqlserver.
 * ============================================================ */
#ifndef GENERADOR_DESTINO_H
#define GENERADOR_DESTINO_H

/* Adapta el SQL al dialecto. Devuelve cadena dinamica.
   'dialecto' puede ser NULL/"" -> se usa "estandar". */
char *generar_codigo_destino(const char *sql, const char *dialecto);

/* Nombre legible del dialecto ya normalizado (para el JSON). */
const char *dialecto_nombre(const char *dialecto);

#endif /* GENERADOR_DESTINO_H */
