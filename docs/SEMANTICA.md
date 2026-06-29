# SEMANTICA.md — ConsultaLang

El análisis semántico (`src/semantica.c`) valida el AST antes de generar SQL.

## Validaciones por instrucción

### SELECT
- No se permite SELECT sin tabla.
- La lista de columnas no puede estar vacía (salvo `todos` / `todo` → `*`).

### INSERT
- No se permite INSERT sin tabla.
- No se permite INSERT sin valores.
- Cada asignación debe tener columna y valor.

### UPDATE
- No se permite UPDATE sin tabla.
- No se permite UPDATE sin campos a modificar.
- Cada asignación debe tener columna y valor.

### DELETE
- No se permite DELETE sin tabla.
- Si DELETE no tiene WHERE, se genera una **advertencia** (no bloquea la traducción).

### CREATE TABLE
- No se permite CREATE TABLE sin nombre de tabla.
- No se permite CREATE TABLE sin columnas.
- No puede haber más de una clave primaria.
- No puede haber columnas repetidas.

## Errores semánticos (ejemplos)

```txt
Error semantico: la tabla no fue especificada.
Error semantico: la lista de columnas no puede estar vacia.
Error semantico: INSERT sin valores.
Error semantico: columna repetida 'id' en la tabla 'clientes'.
Error semantico: la tabla 'clientes' tiene mas de una clave primaria.
```

## Advertencias (ejemplos)

```txt
Advertencia semantica: DELETE sin WHERE puede eliminar todos los registros.
```

## Comportamiento

- Si hay **error** semántico: `semantico = "error"`, `traduccion = "no ejecutado"`, no se genera SQL.
- Si solo hay **advertencias**: `semantico = "correcto"`, se genera SQL y se muestra la advertencia.
