# TRADUCCION.md — ConsultaLang a SQL

El generador (`src/generador.c`) recorre el AST validado y produce SQL estándar.

## Tabla de equivalencias

| ConsultaLang | SQL |
|--------------|-----|
| obtener / seleccionar / listar / traer | SELECT |
| todos / todo | * |
| desde / de / en | FROM |
| donde / cuando / con | WHERE |
| y / o | AND / OR |
| ordenar por | ORDER BY |
| ascendente / descendente | ASC / DESC |
| limite | LIMIT |
| insertar / agregar | INSERT INTO |
| actualizar / modificar | UPDATE |
| establecer / poner | SET |
| eliminar / borrar | DELETE |
| crear tabla | CREATE TABLE |
| entero / texto / decimal / fecha / booleano | INTEGER / VARCHAR(255) / NUMERIC(10,2) / DATE / BOOLEAN |
| primario / obligatorio / unico | PRIMARY KEY / NOT NULL / UNIQUE |
| verdadero / falso | TRUE / FALSE |
| `"texto"` | `'texto'` (comillas simples) |

## Ejemplos de entrada y salida

### SELECT completo
```txt
obtener nombre, edad desde clientes donde edad >= 18 y estado = "activo" ordenar por edad descendente limite 10;
```
```sql
SELECT nombre, edad
FROM clientes
WHERE edad >= 18 AND estado = 'activo'
ORDER BY edad DESC
LIMIT 10;
```

### INSERT
```txt
insertar en clientes nombre = "Juan", edad = 20, estado = "activo";
```
```sql
INSERT INTO clientes (nombre, edad, estado)
VALUES ('Juan', 20, 'activo');
```

### UPDATE
```txt
actualizar clientes establecer estado = "inactivo" donde id = 5;
```
```sql
UPDATE clientes
SET estado = 'inactivo'
WHERE id = 5;
```

### DELETE
```txt
eliminar de clientes donde id = 10;
```
```sql
DELETE FROM clientes
WHERE id = 10;
```

### CREATE TABLE
```txt
crear tabla clientes {
    id entero primario;
    nombre texto obligatorio;
    edad entero;
    correo texto unico;
}
```
```sql
CREATE TABLE clientes (
    id INTEGER PRIMARY KEY,
    nombre VARCHAR(255) NOT NULL,
    edad INTEGER,
    correo VARCHAR(255) UNIQUE
);
```
