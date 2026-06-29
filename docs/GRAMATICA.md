# GRAMATICA.md — ConsultaLang

Gramática implementada en `src/parser.y` (BISON). Notación simplificada tipo BNF.

## Programa

```
programa     → instruccion
instruccion  → select_stmt ';'
             | insert_stmt ';'
             | update_stmt ';'
             | delete_stmt ';'
             | create_stmt
```

## SELECT

```
select_stmt  → SELECT_KW select_cols FROM_KW IDENT opt_where opt_order opt_limit
select_cols  → ALL_KW            (todos / todo  → *)
             | col_list
col_list     → IDENT
             | col_list ',' IDENT
opt_where    → ε | WHERE_KW condicion
condicion    → comparacion
             | condicion AND_KW condicion
             | condicion OR_KW  condicion
comparacion  → IDENT op valor
op           → '=' | COMP            (COMP: != < > <= >=)
valor        → INT | DECIMAL | STRING | BOOLVAL | IDENT
opt_order    → ε | ORDER_KW POR IDENT dir
dir          → ε | ASC_KW | DESC_KW  (ε = ASC por defecto)
opt_limit    → ε | LIMIT_KW INT
```

Precedencia: `o` (OR) tiene menor precedencia que `y` (AND).

## INSERT

```
insert_stmt        → INSERT_KW prep IDENT lista_asignaciones
prep               → FROM_KW (en/de) | A_PREP (a)
lista_asignaciones → asignacion
                   | lista_asignaciones ',' asignacion
asignacion         → IDENT '=' valor
```

## UPDATE

```
update_stmt → UPDATE_KW IDENT SET_KW lista_asignaciones opt_where
```

## DELETE

```
delete_stmt → DELETE_KW FROM_KW IDENT opt_where
```

## CREATE TABLE

```
create_stmt         → CREAR TABLA IDENT '{' lista_coldefs '}'
lista_coldefs       → coldef
                    | lista_coldefs coldef
coldef              → IDENT TIPO lista_restricciones ';'
lista_restricciones → ε | lista_restricciones restriccion
restriccion         → PRIMARIO | OBLIGATORIO | UNICO
```
