/* ============================================================
 * lexer.l - Analizador lexico de ConsultaLang (FLEX)
 * ------------------------------------------------------------
 * Reconoce palabras del lenguaje en espanol y las convierte en
 * tokens para el parser BISON. Maneja sinonimos y traduce tipos
 * y booleanos al vuelo. Ademas registra cada token (tipo, lexema,
 * linea, columna) para mostrar la tabla de tokens en la interfaz.
 * ============================================================ */
%{
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "errores.h"
#include "parser.tab.h"

/* Seguimiento de columna para la tabla de tokens */
int col_actual = 1;   /* columna del siguiente caracter */
int col_tok = 1;      /* columna donde empieza el token actual */

/* Se ejecuta antes de cada accion: fija la columna de inicio y avanza */
#define YY_USER_ACTION {                                  \
    col_tok = col_actual;                                 \
    for (int _i = 0; _i < yyleng; _i++) {                 \
        if (yytext[_i] == '\n') col_actual = 1;           \
        else col_actual++;                                \
    }                                                     \
}

/* Registra el token en la tabla de tokens */
#define REG(TP) agregar_token((TP), yytext, yylineno, col_tok)

/* Quita las comillas dobles de un literal de texto "..." */
static char *quitar_comillas(const char *s) {
    size_t n = strlen(s);
    char *r = (char *)malloc(n);
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
"la"|"el"|"los"|"las"|"un"|"una"|"campos"|"campo"|"registros"|"registro"   { /* relleno */ }

 /* ----- Palabras clave SELECT ----- */
"obtener"|"seleccionar"|"listar"|"traer"|"muestra"|"muestrame"|"dame"|"ver"   { REG("PALABRA_CLAVE"); return SELECT_KW; }
"todos"|"todo"|"todas"|"toda"              { REG("PALABRA_CLAVE"); return ALL_KW; }

 /* ----- FROM y preposiciones ----- */
"desde"|"de"|"en"        { REG("PALABRA_CLAVE"); return FROM_KW; }
"a"                      { REG("PALABRA_CLAVE"); return A_PREP; }

 /* ----- WHERE ----- */
"donde"|"cuando"|"con"   { REG("PALABRA_CLAVE"); return WHERE_KW; }

 /* ----- Operadores logicos ----- */
"y"                      { REG("PALABRA_CLAVE"); return AND_KW; }
"o"                      { REG("PALABRA_CLAVE"); return OR_KW; }

 /* ----- ORDER BY ----- */
"ordenar"|"ordenado"     { REG("PALABRA_CLAVE"); return ORDER_KW; }
"por"                    { REG("PALABRA_CLAVE"); return POR; }
"ascendente"             { REG("PALABRA_CLAVE"); return ASC_KW; }
"descendente"            { REG("PALABRA_CLAVE"); return DESC_KW; }

 /* ----- LIMIT ----- */
"limite"                 { REG("PALABRA_CLAVE"); return LIMIT_KW; }

 /* ----- DISTINCT / GROUP BY / HAVING ----- */
"distinto"|"distintos"   { REG("PALABRA_CLAVE"); return DISTINCT_KW; }
"agrupar"                { REG("PALABRA_CLAVE"); return AGRUPAR; }
"teniendo"               { REG("PALABRA_CLAVE"); return TENIENDO; }

 /* ----- JOIN ----- */
"unir"                   { REG("PALABRA_CLAVE"); return UNIR; }
"interna"                { REG("PALABRA_CLAVE"); return INTERNA; }
"izquierda"              { REG("PALABRA_CLAVE"); return IZQUIERDA; }
"derecha"                { REG("PALABRA_CLAVE"); return DERECHA; }

 /* ----- Funciones de agregacion (se traducen a SQL) ----- */
"contar"     { REG("FUNCION"); yylval.str = cl_strdup("COUNT"); return AGG; }
"sumar"      { REG("FUNCION"); yylval.str = cl_strdup("SUM");   return AGG; }
"promedio"   { REG("FUNCION"); yylval.str = cl_strdup("AVG");   return AGG; }
"minimo"     { REG("FUNCION"); yylval.str = cl_strdup("MIN");   return AGG; }
"maximo"     { REG("FUNCION"); yylval.str = cl_strdup("MAX");   return AGG; }

 /* ----- Condiciones especiales ----- */
"entre"      { REG("PALABRA_CLAVE"); return ENTRE; }
"como"       { REG("PALABRA_CLAVE"); return COMO; }
"es"         { REG("PALABRA_CLAVE"); return ES; }
"nulo"       { REG("PALABRA_CLAVE"); return NULO; }
"no"         { REG("PALABRA_CLAVE"); return NO; }

 /* ----- INSERT / UPDATE / DELETE ----- */
"insertar"|"inserta"|"agrega"|"anade"|"a\xC3\xB1ade"             { REG("PALABRA_CLAVE"); return INSERT_KW; }
"actualizar"|"modificar"|"actualiza"|"modifica"|"cambia"         { REG("PALABRA_CLAVE"); return UPDATE_KW; }
"establecer"|"poner"|"poniendo"|"estableciendo"                  { REG("PALABRA_CLAVE"); return SET_KW; }
"eliminar"|"borrar"|"elimina"|"borra"|"quita"                    { REG("PALABRA_CLAVE"); return DELETE_KW; }

 /* ----- DDL: CREATE / ALTER / DROP TABLE ----- */
"crear"|"crea"|"definir"|"define"   { REG("PALABRA_CLAVE"); return CREAR; }
"alterar"|"altera"                  { REG("PALABRA_CLAVE"); return ALTERAR; }
"agregar"                           { REG("PALABRA_CLAVE"); return AGREGAR; }
"tabla"                             { REG("PALABRA_CLAVE"); return TABLA; }

 /* ----- Tipos de datos (traducidos a SQL) ----- */
"entero"     { REG("TIPO"); yylval.str = cl_strdup("INTEGER");       return TIPO; }
"texto"      { REG("TIPO"); yylval.str = cl_strdup("VARCHAR(255)");  return TIPO; }
"decimal"    { REG("TIPO"); yylval.str = cl_strdup("NUMERIC(10,2)"); return TIPO; }
"fecha"      { REG("TIPO"); yylval.str = cl_strdup("DATE");          return TIPO; }
"booleano"   { REG("TIPO"); yylval.str = cl_strdup("BOOLEAN");       return TIPO; }

 /* ----- Restricciones de columna ----- */
"clave"        { REG("PALABRA_CLAVE"); return CLAVE; }
"primaria"     { REG("PALABRA_CLAVE"); return PRIMARIA; }
"primario"     { REG("PALABRA_CLAVE"); return PRIMARIO; }
"obligatorio"  { REG("PALABRA_CLAVE"); return OBLIGATORIO; }
"unico"        { REG("PALABRA_CLAVE"); return UNICO; }

 /* ----- Valores booleanos ----- */
"verdadero"  { REG("BOOLEANO"); yylval.val = nuevoValue(V_BOOL, "TRUE");  return BOOLVAL; }
"falso"      { REG("BOOLEANO"); yylval.val = nuevoValue(V_BOOL, "FALSE"); return BOOLVAL; }

 /* ----- Operadores de comparacion ----- */
"="                      { REG("OPERADOR"); return '='; }
"!="|"<>"                { REG("OPERADOR"); yylval.str = cl_strdup("!="); return COMP; }
"<="                     { REG("OPERADOR"); yylval.str = cl_strdup("<="); return COMP; }
">="                     { REG("OPERADOR"); yylval.str = cl_strdup(">="); return COMP; }
"<"                      { REG("OPERADOR"); yylval.str = cl_strdup("<");  return COMP; }
">"                      { REG("OPERADOR"); yylval.str = cl_strdup(">");  return COMP; }

 /* ----- Signos de puntuacion ----- */
","   { REG("SIMBOLO"); return ','; }
";"   { REG("SIMBOLO"); return ';'; }
"{"   { REG("SIMBOLO"); return '{'; }
"}"   { REG("SIMBOLO"); return '}'; }
"("   { REG("SIMBOLO"); return '('; }
")"   { REG("SIMBOLO"); return ')'; }
"*"   { REG("SIMBOLO"); return '*'; }

 /* ----- Valores numericos ----- */
{DIGITO}+"."{DIGITO}+    { REG("DECIMAL"); yylval.val = nuevoValue(V_DECIMAL, yytext); return DECIMAL; }
{DIGITO}+                { REG("NUMERO");  yylval.val = nuevoValue(V_INT, yytext);     return INT; }

 /* ----- Texto entre comillas ----- */
\"[^\"\n]*\"             {
                            REG("CADENA");
                            char *inner = quitar_comillas(yytext);
                            yylval.val = nuevoValue(V_STRING, inner);
                            free(inner);
                            return STRING;
                         }

 /* ----- Identificadores (admite calificados: c.nombre) ----- */
{LETRA}{IDCHAR}*("."{LETRA}{IDCHAR}*)?   { REG("IDENTIFICADOR"); yylval.str = cl_strdup(yytext); return IDENT; }

 /* ----- Cualquier otro simbolo: error lexico ----- */
.                        {
                            reporte.lexico = 0;
                            agregar_error("Error lexico en linea %d: simbolo no reconocido '%s'",
                                          yylineno, yytext);
                         }

%%
