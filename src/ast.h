/* ============================================================
 * ast.h - Arbol de Sintaxis Abstracta (AST) de ConsultaLang
 * ------------------------------------------------------------
 * El parser (BISON) construye el AST; luego lo usan el analisis
 * semantico, el generador de SQL y el generador de codigo destino.
 * ============================================================ */
#ifndef AST_H
#define AST_H

/* Tipo de instruccion principal */
typedef enum {
    ST_SELECT,
    ST_INSERT,
    ST_UPDATE,
    ST_DELETE,
    ST_CREATE,
    ST_ALTER,
    ST_DROP
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

/* Lista enlazada de valores (para IN (...)) */
typedef struct ValList {
    Value *val;
    struct ValList *next;
} ValList;

/* ---------- Lista de columnas (SELECT) ---------- */
typedef struct ColItem {
    char *name;              /* "nombre", "c.nombre", "COUNT(*)"... */
    struct ColItem *next;
} ColItem;

/* ---------- Condiciones (WHERE / HAVING / ON) ---------- */
typedef enum {
    C_CMP,        /* col op valor            */
    C_AND,
    C_OR,
    C_BETWEEN,    /* col BETWEEN v1 AND v2   */
    C_IN,         /* col IN (lista)          */
    C_LIKE,       /* col LIKE valor          */
    C_ISNULL,     /* col IS NULL             */
    C_ISNOTNULL   /* col IS NOT NULL         */
} CondType;

typedef struct Cond {
    CondType type;
    /* C_CMP / especiales: lado izquierdo y operador */
    char *col;
    char *op;
    Value *val;          /* valor principal (CMP, LIKE, BETWEEN v1) */
    Value *val2;         /* BETWEEN v2 */
    ValList *inlist;     /* IN (...) */
    /* C_AND / C_OR */
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

/* ---------- JOIN ---------- */
typedef struct Join {
    char *tipo;      /* "JOIN", "INNER JOIN", "LEFT JOIN", "RIGHT JOIN" */
    char *table;
    char *alias;     /* puede ser NULL */
    Cond *on;        /* condicion ON */
    struct Join *next;
} Join;

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
    char *alias;         /* alias de la tabla principal (FROM ... c) */

    /* SELECT */
    int distinct;        /* SELECT DISTINCT */
    int select_all;      /* 1 = SELECT *      */
    ColItem *columns;    /* lista de columnas */
    Join *joins;         /* JOINs             */
    Cond *where;         /* condicion WHERE   */
    ColItem *group_by;   /* GROUP BY columnas */
    Cond *having;        /* HAVING            */
    OrderBy *order;      /* ORDER BY (NULL si no hay) */
    int limit;           /* LIMIT (-1 si no hay)      */

    /* INSERT / UPDATE */
    Assign *assigns;

    /* CREATE / ALTER TABLE */
    ColDef *coldefs;
} Stmt;

/* ---------- Constructores (definidos en ast.c) ---------- */
Stmt    *nuevoStmt(StmtType type);
Value   *nuevoValue(ValueType type, const char *text);
ValList *nuevoValList(Value *v);
ValList *agregarValList(ValList *lista, Value *v);
ColItem *nuevaColumna(const char *name);
ColItem *agregarColumna(ColItem *lista, ColItem *nueva);
Cond    *nuevaComparacion(const char *col, const char *op, Value *val);
Cond    *nuevaLogica(CondType type, Cond *left, Cond *right);
Cond    *nuevoBetween(const char *col, Value *v1, Value *v2);
Cond    *nuevoIn(const char *col, ValList *lista);
Cond    *nuevoLike(const char *col, Value *val);
Cond    *nuevoIsNull(const char *col, int negado);
Assign  *nuevaAsignacion(const char *col, Value *val);
Assign  *agregarAsignacion(Assign *lista, Assign *nueva);
OrderBy *nuevoOrden(const char *col, const char *dir);
Join    *nuevoJoin(const char *tipo, const char *table, const char *alias, Cond *on);
Join    *agregarJoin(Join *lista, Join *nuevo);
ColDef  *nuevaColDef(const char *name, const char *type);
ColDef  *agregarColDef(ColDef *lista, ColDef *nueva);

/* Duplicar cadena (helper portable) */
char *cl_strdup(const char *s);

/* Raiz global que llena el parser */
extern Stmt *raiz;

#endif /* AST_H */
