/* ============================================================
 * ast.h - Arbol de Sintaxis Abstracta (AST) de ConsultaLang
 * ------------------------------------------------------------
 * Define las estructuras que el parser (BISON) construye y que
 * luego usan el analizador semantico y el generador de SQL.
 * Mantener el AST separado hace el codigo facil de explicar:
 *   parser  -> construye AST
 *   semantica -> valida AST
 *   generador -> AST a SQL
 * ============================================================ */
#ifndef AST_H
#define AST_H

/* Tipo de instruccion principal */
typedef enum {
    ST_SELECT,
    ST_INSERT,
    ST_UPDATE,
    ST_DELETE,
    ST_CREATE
} StmtType;

/* ---------- Valores literales ---------- */
typedef enum {
    V_INT,      /* 10            */
    V_DECIMAL,  /* 5000.50       */
    V_STRING,   /* "activo"      */
    V_BOOL,     /* verdadero/falso */
    V_IDENT     /* identificador */
} ValueType;

typedef struct Value {
    ValueType type;
    char *text;   /* texto ya normalizado para SQL */
} Value;

/* ---------- Lista de columnas (SELECT) ---------- */
typedef struct ColItem {
    char *name;
    struct ColItem *next;
} ColItem;

/* ---------- Condiciones (WHERE) ---------- */
typedef enum { C_CMP, C_AND, C_OR } CondType;

typedef struct Cond {
    CondType type;
    /* Comparacion simple (C_CMP): col op val */
    char *col;
    char *op;
    Value *val;
    /* Logica (C_AND / C_OR) */
    struct Cond *left;
    struct Cond *right;
} Cond;

/* ---------- Asignaciones col = valor (INSERT / UPDATE) ---------- */
typedef struct Assign {
    char *col;
    Value *val;
    struct Assign *next;
} Assign;

/* ---------- Orden (ORDER BY) ---------- */
typedef struct OrderBy {
    char *col;
    char *dir;   /* "ASC" o "DESC" */
} OrderBy;

/* ---------- Definicion de columna (CREATE TABLE) ---------- */
typedef struct ColDef {
    char *name;
    char *type;     /* tipo ya traducido: INTEGER, VARCHAR(255)... */
    int primary;    /* PRIMARY KEY */
    int notnull;    /* NOT NULL    */
    int unique;     /* UNIQUE      */
    struct ColDef *next;
} ColDef;

/* ---------- Nodo raiz de una instruccion ---------- */
typedef struct {
    StmtType type;
    char *table;

    /* SELECT */
    int select_all;      /* 1 = SELECT *      */
    ColItem *columns;    /* lista de columnas */
    Cond *where;         /* condicion WHERE   */
    OrderBy *order;      /* ORDER BY (NULL si no hay) */
    int limit;           /* LIMIT (-1 si no hay)      */

    /* INSERT / UPDATE */
    Assign *assigns;

    /* CREATE TABLE */
    ColDef *coldefs;
} Stmt;

/* ---------- Constructores (definidos en ast.c) ---------- */
Stmt    *nuevoStmt(StmtType type);
Value   *nuevoValue(ValueType type, const char *text);
ColItem *nuevaColumna(const char *name);
ColItem *agregarColumna(ColItem *lista, ColItem *nueva);
Cond    *nuevaComparacion(const char *col, const char *op, Value *val);
Cond    *nuevaLogica(CondType type, Cond *left, Cond *right);
Assign  *nuevaAsignacion(const char *col, Value *val);
Assign  *agregarAsignacion(Assign *lista, Assign *nueva);
OrderBy *nuevoOrden(const char *col, const char *dir);
ColDef  *nuevaColDef(const char *name, const char *type);
ColDef  *agregarColDef(ColDef *lista, ColDef *nueva);

/* Duplicar cadena (helper portable) */
char *cl_strdup(const char *s);

/* Raiz global que llena el parser */
extern Stmt *raiz;

#endif /* AST_H */
