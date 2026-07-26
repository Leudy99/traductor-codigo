# TABLA_SIMBOLOS.md — Tabla de símbolos (dinámica)

## Qué es

La **tabla de símbolos** registra los símbolos realmente usados en la instrucción
que escribe el usuario. **No** es una lista fija de tablas/columnas: se **regenera
en cada análisis** recorriendo el AST.

> Importante: la *tabla de símbolos* es distinta de la *tabla de tokens* del
> análisis léxico. Los tokens son las unidades léxicas (palabra clave, identificador,
> número…); la tabla de símbolos describe el uso semántico de cada símbolo.

Implementada en `src/tabla_simbolos.c`:

```c
void construir_tabla_simbolos(Stmt *s);
```

## Columnas

| Columna | Significado |
|---------|-------------|
| Símbolo | Nombre del símbolo (tabla, columna, alias, literal, función) |
| Categoría | tabla / columna / alias / literal / función |
| Tipo/Clase | identificador, entero, cadena, booleano, agregación, el tipo de dato… |
| Ámbito/Tabla | dónde vive (global, la tabla o el alias asociado) |
| Información | contexto de uso (FROM, SELECT, WHERE, JOIN, ON, GROUP BY…) |

## Comportamiento

- Se **limpia** antes de cada análisis (`reporte_init`).
- Registra **solo** símbolos de la entrada actual.
- **Evita duplicados**: si un símbolo aparece en más de un contexto, fusiona la
  información (ej. `SELECT / WHERE`).
- Registra tablas, alias, columnas, literales y funciones de agregación.

## Ejemplo

Entrada:
```txt
obtener c.nombre, v.total desde clientes c unir ventas v en c.id = v.id_cliente donde v.total >= 500;
```

Tabla de símbolos:

| Símbolo | Categoría | Tipo/Clase | Ámbito/Tabla | Información |
|---------|-----------|------------|--------------|-------------|
| clientes | tabla | identificador | global | FROM |
| c | alias | tabla | clientes | alias |
| ventas | tabla | identificador | global | JOIN |
| v | alias | tabla | ventas | alias |
| id | columna | identificador | c | ON |
| id_cliente | columna | identificador | v | ON |
| nombre | columna | identificador | c | SELECT |
| total | columna | identificador | v | SELECT / WHERE |
| 500 | literal | entero | WHERE | valor |

## Salida JSON

```json
"tabla_simbolos": [
  { "simbolo": "clientes", "categoria": "tabla", "tipo": "identificador", "ambito": "global", "info": "FROM" },
  ...
]
```

## Limitación

Sin un catálogo/esquema real de la base de datos, la semántica valida **alias y
prefijos calificados**, pero no comprueba si una columna existe físicamente en la
tabla (esquema académico interno).
