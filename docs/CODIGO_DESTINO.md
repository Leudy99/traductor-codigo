# CODIGO_DESTINO.md — Generador de código destino

## Qué es

Es la **fase final** del traductor. Toma el SQL ya optimizado y lo adapta a cada
**dialecto SQL**. No hay que elegir nada: se muestran **los cuatro a la vez**.

```txt
... → Generación SQL bruto → Optimización → Generador de código destino → SQL destino final
```

Implementado en `src/generador_destino.c`:

```c
char *generar_codigo_destino(const char *sql, const char *dialecto);
```

## Dialectos soportados

- **SQL estándar**
- **MySQL**
- **PostgreSQL**
- **SQL Server**

Los cuatro se generan siempre y se muestran juntos en la interfaz.

## Transformaciones que realiza

| Dialecto | Transformación |
|----------|----------------|
| SQL estándar | Ninguna (conserva LIMIT) |
| MySQL | Conserva LIMIT |
| PostgreSQL | Conserva LIMIT |
| SQL Server | Convierte `LIMIT N` en `SELECT TOP N ...` |

### Ejemplo (SQL Server)

Entrada ConsultaLang:
```txt
obtener nombre, edad desde clientes donde edad >= 18 limite 10;
```

SQL optimizado (estándar):
```sql
SELECT nombre, edad FROM clientes WHERE edad >= 18 LIMIT 10;
```

SQL destino (SQL Server):
```sql
SELECT TOP 10 nombre, edad FROM clientes WHERE edad >= 18;
```

Además se muestra la advertencia:
`SQL Server: 'LIMIT N' se convirtio a 'SELECT TOP N'.`

## Salida JSON

```json
"codigo_destino": [
  { "dialecto": "SQL estandar", "sql": "...", "advertencias": [] },
  { "dialecto": "MySQL",        "sql": "...", "advertencias": [] },
  { "dialecto": "PostgreSQL",   "sql": "...", "advertencias": [] },
  { "dialecto": "SQL Server",   "sql": "...", "advertencias": ["..."] }
]
```

## Limitaciones

- Conversión centrada en `LIMIT`/`TOP` (lo más didáctico).
- No traduce tipos de datos por dialecto (INTEGER, VARCHAR se dejan igual).
- Mantiene compatibilidad con SELECT (incl. JOIN), INSERT, UPDATE, DELETE,
  CREATE TABLE, ALTER TABLE y DROP TABLE.
- No maneja OFFSET ni paginación avanzada.
