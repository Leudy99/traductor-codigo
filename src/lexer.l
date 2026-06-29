/* ============================================================
 * lexer.l - Analizador lexico de ConsultaLang (FLEX)
 * ------------------------------------------------------------
 * Reconoce palabras del lenguaje en espanol y las convierte en
 * tokens para el parser BISON. Tambien maneja sinonimos
 * (obtener/listar/seleccionar/traer -> SELECT_KW) y traduce
 * tipos y booleanos al vuelo.
 * ============================================================ */
%{
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "errores.h"
#include "parser.tab.h"

/* Quita las comillas dobles de un literal de texto "..." */
static char *quitar_comillas(const char *s) {
    size_t n = strlen(s);
    char *r = (char *)malloc(n);     /* n-2 chars + '\0' cabe en n */
    if (n >= 2) {
        memcpy(r, s + 1, n - 2);
        r[n - 2] = '\0';
    } else {
        r[0] = '\0';
    }
    return r;
}
%}

%option noyywrap
%option yylineno
%option never-interactive

DIGITO   [0-9]
LETRA    [A-Za-z_\x80-\xff]
IDCHAR   [A-Za-z0-9_\x80-\xff]

%%

\xEF\xBB\xBF            { /* BOM UTF-8 al inicio: ignorar */ }
[ \t\r\n]+              { /* espacios: ignorar */ }
"--".*                  { /* comentario de linea: ignorar */ }

 /* ----- Palabras de relleno (articulos y conectores naturales): se ignoran ----- */
"la"|"el"|"los"|"las"|"un"|"una"|"campos"|"campo"|"registros"|"registro"   { /* relleno: ignorar */ }

 /* ----- Palabras clave SELECT ----- */
"obtener"|"seleccionar"|"listar"|"traer"|"muestra"|"muestrame"|"dame"|"ver"   { return SELECT_KW; }
"todos"|"todo"|"todas"|"toda"              { return ALL_KW; }

 /* ----- FROM y preposiciones ----- */
"desde"|"de"|"en"        { return FROM_KW; }
"a"                      { return A_PREP; }

 /* ----- WHERE ----- */
"donde"|"cuando"|"con"   { return WHERE_KW; }

 /* ----- Operadores logicos ----- */
"y"                      { return AND_KW; }
"o"                      { return OR_KW; }

 /* ----- ORDER BY ----- */
"ordenar"|"ordenado"     { return ORDER_KW; }
"por"                    { return POR; }
"ascendente"             { return ASC_KW; }
"descendente"            { return DESC_KW; }

 /* ----- LIMIT ----- */
"limite"                 { return LIMIT_KW; }

 /* ----- INSERT / UPDATE / DELETE ----- */
"insertar"|"agregar"|"inserta"|"agrega"|"anade"|"a\xC3\xB1ade"   { return INSERT_KW; }
"actualizar"|"modificar"|"actualiza"|"modifica"|"cambia"         { return UPDATE_KW; }
"establecer"|"poner"|"poniendo"|"estableciendo"                  { return SET_KW; }
"eliminar"|"borrar"|"elimina"|"borra"|"quita"                    { return DELETE_KW; }

 /* ----- CREATE TABLE ----- */
"crear"|"crea"|"definir"|"define"   { return CREAR; }
"tabla"                             { return TABLA; }

 /* ----- Tipos de datos (traducidos a SQL) ----- */
"entero"     { yylval.str = cl_strdup("INTEGER");       return TIPO; }
"texto"      { yylval.str = cl_strdup("VARCHAR(255)");  return TIPO; }
"decimal"    { yylval.str = cl_strdup("NUMERIC(10,2)"); return TIPO; }
"fecha"      { yylval.str = cl_strdup("DATE");          return TIPO; }
"booleano"   { yylval.str = cl_strdup("BOOLEAN");       return TIPO; }

 /* ----- Restricciones de columna ----- */
"primario"     { return PRIMARIO; }
"obligatorio"  { return OBLIGATORIO; }
"unico"        { return UNICO; }

 /* ----- Valores booleanos ----- */
"verdadero"  { yylval.val = nuevoValue(V_BOOL, "TRUE");  return BOOLVAL; }
"falso"      { yylval.val = nuevoValue(V_BOOL, "FALSE"); return BOOLVAL; }

 /* ----- Operadores de comparacion ----- */
"="                      { return '='; }
"!="|"<>"                { yylval.str = cl_strdup("!="); return COMP; }
"<="                     { yylval.str = cl_strdup("<="); return COMP; }
">="                     { yylval.str = cl_strdup(">="); return COMP; }
"<"                      { yylval.str = cl_strdup("<");  return COMP; }
">"                      { yylval.str = cl_strdup(">");  return COMP; }

 /* ----- Signos de puntuacion ----- */
","   { return ','; }
";"   { return ';'; }
"{"   { return '{'; }
"}"   { return '}'; }

 /* ----- Valores numericos ----- */
{DIGITO}+"."{DIGITO}+    { yylval.val = nuevoValue(V_DECIMAL, yytext); return DECIMAL; }
{DIGITO}+                { yylval.val = nuevoValue(V_INT, yytext);     return INT; }

 /* ----- Texto entre comillas ----- */
\"[^\"\n]*\"             {
                            char *inner = quitar_comillas(yytext);
                            yylval.val = nuevoValue(V_STRING, inner);
                            free(inner);
                            return STRING;
                         }

 /* ----- Identificadores ----- */
{LETRA}{IDCHAR}*         { yylval.str = cl_strdup(yytext); return IDENT; }

 /* ----- Cualquier otro simbolo: error lexico ----- */
.                        {
                            reporte.lexico = 0;
                            agregar_error("Error lexico en linea %d: simbolo no reconocido '%s'",
                                          yylineno, yytext);
                         }

%%
