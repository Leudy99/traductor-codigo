/* ============================================================
 * parser.y - Analizador sintactico de ConsultaLang (BISON)
 * ------------------------------------------------------------
 * Define la gramatica del lenguaje y construye el AST.
 * Soporta: SELECT, INSERT, UPDATE, DELETE y CREATE TABLE.
 * ============================================================ */
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "errores.h"

int yylex(void);
extern int yylineno;
void yyerror(const char *s);
%}

/* Mensajes de error detallados (indican que se esperaba) */
%define parse.error verbose

/* Valores semanticos posibles de tokens y reglas */
%union {
    char    *str;
    int      num;
    struct Value   *val;
    struct Cond    *cond;
    struct ColItem *col;
    struct Assign  *asg;
    struct ColDef  *cdef;
    struct OrderBy *ord;
}

/* ----- Tokens con valor (con nombre legible para los errores) ----- */
%token <str> IDENT  "identificador"
%token <str> COMP   "operador de comparacion"
%token <str> TIPO   "tipo de dato"
%token <val> INT     "numero entero"
%token <val> DECIMAL "numero decimal"
%token <val> STRING  "texto entre comillas"
%token <val> BOOLVAL "valor booleano"

/* ----- Palabras clave (con nombre legible) ----- */
%token SELECT_KW "obtener"
%token ALL_KW    "todos"
%token FROM_KW   "desde"
%token A_PREP    "a"
%token WHERE_KW  "donde"
%token AND_KW    "y"
%token OR_KW     "o"
%token ORDER_KW  "ordenar"
%token POR       "por"
%token ASC_KW    "ascendente"
%token DESC_KW   "descendente"
%token LIMIT_KW  "limite"
%token INSERT_KW "insertar"
%token UPDATE_KW "actualizar"
%token SET_KW    "establecer"
%token DELETE_KW "eliminar"
%token CREAR     "crear"
%token TABLA     "tabla"
%token PRIMARIO  "primario"
%token OBLIGATORIO "obligatorio"
%token UNICO     "unico"

/* ----- Tipos de las reglas ----- */
%type <col>  col_list select_cols
%type <cond> condicion comparacion opt_where
%type <str>  op dir
%type <val>  valor
%type <asg>  asignacion lista_asignaciones
%type <ord>  opt_order
%type <num>  opt_limit restriccion lista_restricciones
%type <cdef> coldef lista_coldefs coldef_nat lista_coldefs_nat

/* Precedencia: OR mas debil que AND */
%left OR_KW
%left AND_KW

%%

/* ============ Programa ============ */
programa
    : instruccion
    ;

instruccion
    : select_stmt opt_pyc
    | insert_stmt opt_pyc
    | update_stmt opt_pyc
    | delete_stmt opt_pyc
    | create_stmt opt_pyc
    ;

/* El ';' final es opcional */
opt_pyc
    : /* vacio */
    | ';'
    ;

/* ============ SELECT ============ */
select_stmt
    : SELECT_KW select_cols opt_from IDENT opt_where opt_order opt_limit
        {
            Stmt *s = nuevoStmt(ST_SELECT);
            if ($2 == NULL) s->select_all = 1;   /* todos/todo */
            else            s->columns = $2;
            s->table = cl_strdup($4);
            s->where = $5;
            s->order = $6;
            s->limit = $7;
            raiz = s;
        }
    ;

/* 'desde/de/en' es opcional (forma natural: "muestra todos los clientes") */
opt_from
    : /* vacio */
    | FROM_KW
    ;

select_cols
    : ALL_KW            { $$ = NULL; }   /* NULL = SELECT * */
    | col_list          { $$ = $1; }
    ;

col_list
    : IDENT                 { $$ = nuevaColumna($1); }
    | col_list ',' IDENT    { $$ = agregarColumna($1, nuevaColumna($3)); }
    ;

opt_where
    : /* vacio */               { $$ = NULL; }
    | WHERE_KW condicion        { $$ = $2; }
    ;

condicion
    : comparacion                       { $$ = $1; }
    | condicion AND_KW condicion        { $$ = nuevaLogica(C_AND, $1, $3); }
    | condicion OR_KW condicion         { $$ = nuevaLogica(C_OR, $1, $3); }
    ;

comparacion
    : IDENT op valor    { $$ = nuevaComparacion($1, $2, $3); }
    ;

op
    : '='       { $$ = cl_strdup("="); }
    | COMP      { $$ = $1; }
    ;

valor
    : INT       { $$ = $1; }
    | DECIMAL   { $$ = $1; }
    | STRING    { $$ = $1; }
    | BOOLVAL   { $$ = $1; }
    | IDENT     { $$ = nuevoValue(V_IDENT, $1); }
    ;

opt_order
    : /* vacio */                       { $$ = NULL; }
    | ORDER_KW POR IDENT dir            { $$ = nuevoOrden($3, $4); }
    ;

dir
    : /* vacio */   { $$ = cl_strdup("ASC"); }   /* ascendente por defecto */
    | ASC_KW        { $$ = cl_strdup("ASC"); }
    | DESC_KW       { $$ = cl_strdup("DESC"); }
    ;

opt_limit
    : /* vacio */       { $$ = -1; }
    | LIMIT_KW INT      { $$ = atoi($2->text); }
    ;

/* ============ INSERT ============ */
insert_stmt
    : INSERT_KW prep IDENT lista_asignaciones
        {
            Stmt *s = nuevoStmt(ST_INSERT);
            s->table = cl_strdup($3);
            s->assigns = $4;
            raiz = s;
        }
    ;

prep
    : FROM_KW       /* en / de / desde */
    | A_PREP        /* a */
    ;

lista_asignaciones
    : asignacion                            { $$ = $1; }
    | lista_asignaciones ',' asignacion     { $$ = agregarAsignacion($1, $3); }
    ;

asignacion
    : IDENT '=' valor   { $$ = nuevaAsignacion($1, $3); }
    ;

/* ============ UPDATE ============ */
update_stmt
    : UPDATE_KW IDENT SET_KW lista_asignaciones opt_where
        {
            Stmt *s = nuevoStmt(ST_UPDATE);
            s->table = cl_strdup($2);
            s->assigns = $4;
            s->where = $5;
            raiz = s;
        }
    ;

/* ============ DELETE ============ */
delete_stmt
    : DELETE_KW opt_from IDENT opt_where
        {
            Stmt *s = nuevoStmt(ST_DELETE);
            s->table = cl_strdup($3);
            s->where = $4;
            raiz = s;
        }
    ;

/* ============ CREATE TABLE ============ */
create_stmt
    /* Forma con llaves: crear tabla X { col tipo; ... } */
    : CREAR TABLA IDENT '{' lista_coldefs '}'
        {
            Stmt *s = nuevoStmt(ST_CREATE);
            s->table = cl_strdup($3);
            s->coldefs = $5;
            raiz = s;
        }
    /* Forma natural: crea la tabla X con los campos col tipo, col tipo, ... */
    | CREAR TABLA IDENT opt_conector lista_coldefs_nat
        {
            Stmt *s = nuevoStmt(ST_CREATE);
            s->table = cl_strdup($3);
            s->coldefs = $5;
            raiz = s;
        }
    ;

/* Conector opcional antes de los campos ("con", "de") */
opt_conector
    : /* vacio */
    | WHERE_KW      /* con */
    | FROM_KW       /* de / en */
    ;

/* Campos separados por coma, sin llaves ni ';' */
lista_coldefs_nat
    : coldef_nat                        { $$ = $1; }
    | lista_coldefs_nat ',' coldef_nat  { $$ = agregarColDef($1, $3); }
    ;

coldef_nat
    : IDENT TIPO lista_restricciones
        {
            ColDef *c = nuevaColDef($1, $2);
            c->primary = ($3 & 1) ? 1 : 0;
            c->notnull = ($3 & 2) ? 1 : 0;
            c->unique  = ($3 & 4) ? 1 : 0;
            $$ = c;
        }
    ;

lista_coldefs
    : coldef                    { $$ = $1; }
    | lista_coldefs coldef      { $$ = agregarColDef($1, $2); }
    ;

coldef
    : IDENT TIPO lista_restricciones ';'
        {
            ColDef *c = nuevaColDef($1, $2);
            c->primary = ($3 & 1) ? 1 : 0;
            c->notnull = ($3 & 2) ? 1 : 0;
            c->unique  = ($3 & 4) ? 1 : 0;
            $$ = c;
        }
    ;

lista_restricciones
    : /* vacio */                       { $$ = 0; }
    | lista_restricciones restriccion   { $$ = $1 | $2; }
    ;

restriccion
    : PRIMARIO      { $$ = 1; }   /* PRIMARY KEY */
    | OBLIGATORIO   { $$ = 2; }   /* NOT NULL    */
    | UNICO         { $$ = 4; }   /* UNIQUE      */
    ;

%%

/* Manejo de errores sintacticos */
/* Reemplaza todas las apariciones de 'a' por 'b' (src -> dst) */
static void reemplazar(char *dst, size_t n, const char *src,
                       const char *a, const char *b) {
    size_t la = strlen(a), lb = strlen(b), o = 0;
    const char *p = src;
    while (*p && o < n - 1) {
        if (strncmp(p, a, la) == 0) {
            for (size_t i = 0; i < lb && o < n - 1; i++) dst[o++] = b[i];
            p += la;
        } else {
            dst[o++] = *p++;
        }
    }
    dst[o] = '\0';
}

void yyerror(const char *s) {
    /* Si ya hubo error lexico, no duplicamos el mensaje sintactico */
    if (reporte.lexico == 0) return;
    reporte.sintactico = 0;

    /* Traduce las frases que genera BISON al espanol */
    char a[MAX_LEN], b[MAX_LEN];
    reemplazar(a, sizeof(a), s, "syntax error", "error de sintaxis");
    reemplazar(b, sizeof(b), a, ", unexpected", "; no se esperaba");
    reemplazar(a, sizeof(a), b, ", expecting", "; se esperaba");
    reemplazar(b, sizeof(b), a, " or ", " o ");
    reemplazar(a, sizeof(a), b, "$end", "el final de la instruccion");
    reemplazar(b, sizeof(b), a, "end of file", "el final de la instruccion");

    agregar_error("Error sintactico en linea %d: %s", yylineno, b);
}
