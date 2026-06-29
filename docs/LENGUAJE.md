# LENGUAJE.md — ConsultaLang

## ¿Qué es ConsultaLang?

ConsultaLang es un lenguaje de consulta personalizado, escrito en **español**, que se traduce a **SQL estándar**. Permite expresar operaciones de base de datos con palabras naturales en español (obtener, desde, donde, etc.).

## Objetivo del lenguaje

Acercar la sintaxis de las consultas a personas que hablan español, demostrando a la vez las fases de un traductor: léxico, sintáctico, semántico y generación de código.

## Palabras reservadas

| ConsultaLang | SQL |
|--------------|-----|
| obtener, seleccionar, listar, traer | SELECT |
| todos, todo | * |
| desde, de, en | FROM |
| donde, cuando, con | WHERE |
| y | AND |
| o | OR |
| ordenar por, ordenado por | ORDER BY |
| ascendente | ASC |
| descendente | DESC |
| limite | LIMIT |
| insertar, agregar | INSERT INTO |
| actualizar, modificar | UPDATE |
| establecer, poner | SET |
| eliminar, borrar | DELETE |
| crear tabla | CREATE TABLE |
| entero | INTEGER |
| texto | VARCHAR(255) |
| decimal | NUMERIC(10,2) |
| fecha | DATE |
| booleano | BOOLEAN |
| primario | PRIMARY KEY |
| obligatorio | NOT NULL |
| unico | UNIQUE |
| verdadero | TRUE |
| falso | FALSE |

## Tipos de instrucciones soportadas

1. SELECT básico (`obtener todos desde clientes;`)
2. SELECT con columnas
3. SELECT con WHERE
4. Condiciones con `y` / `o`
5. ORDER BY
6. LIMIT
7. INSERT
8. UPDATE
9. DELETE
10. CREATE TABLE

## Valores soportados

- Enteros: `18`
- Decimales: `5000.50`
- Texto entre comillas dobles: `"activo"`
- Booleanos: `verdadero`, `falso`
- Identificadores: `edad`, `nombre`

## Operadores

`=`  `!=`  `<`  `>`  `<=`  `>=`

## Ejemplos de uso

```txt
obtener nombre, edad desde clientes donde edad >= 18;
insertar en clientes nombre = "Juan", edad = 20;
actualizar clientes establecer estado = "inactivo" donde id = 5;
eliminar de clientes donde id = 10;
crear tabla clientes { id entero primario; nombre texto obligatorio; }
```
