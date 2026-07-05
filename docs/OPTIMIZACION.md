# OPTIMIZACION.md — Optimizador de código de ConsultaLang

## 1. Qué es el optimizador

Es una fase adicional del traductor que recibe el **SQL bruto** (el que produce el
generador) y devuelve una versión **optimizada**: más limpia, sin condiciones
redundantes y con simplificaciones simples sobre la cláusula `WHERE`.

## 2. En qué fase se ejecuta

```txt
Entrada ConsultaLang
  → Léxico
  → Sintáctico
  → Semántico
  → Generación de SQL bruto
  → Optimización de código   ← nueva fase
  → SQL optimizado
```

Se implementa en `src/optimizador.c` con la función:

```c
char *optimizar_sql(const char *sql_original);
```

Trabaja sobre el texto del SQL, aislando la cláusula `WHERE`, separándola en
predicados y conectores (`AND` / `OR`) y aplicando las reglas.

## 3. Reglas que aplica

| # | Regla | Efecto |
|---|-------|--------|
| 1 | Condición repetida (AND) | Elimina el predicado duplicado |
| 2 | OR repetido | Elimina el predicado OR duplicado |
| 3 | OR sobre el mismo campo | Convierte a `campo IN (...)` |
| 4 | Contradicción (rango imposible) | Solo emite una advertencia (no altera el SQL) |
| 5 | Normalización | Ajusta espacios del SQL final |

## 4. Ejemplos de SQL bruto y optimizado

### Regla 1 — condición repetida
```txt
obtener nombre desde clientes donde edad >= 18 y edad >= 18;
```
Bruto: `SELECT nombre FROM clientes WHERE edad >= 18 AND edad >= 18;`
Optimizado: `SELECT nombre FROM clientes WHERE edad >= 18;`

### Regla 2 — OR repetido
```txt
obtener nombre desde clientes donde estado = "activo" o estado = "activo";
```
Optimizado: `SELECT nombre FROM clientes WHERE estado = 'activo';`

### Regla 3 — OR → IN
```txt
obtener nombre, correo desde clientes donde estado = "activo" o estado = "pendiente" o estado = "suspendido";
```
Optimizado: `SELECT nombre, correo FROM clientes WHERE estado IN ('activo', 'pendiente', 'suspendido');`

### Regla 4 — contradicción (solo advertencia)
```txt
obtener nombre desde empleados donde edad >= 65 y edad <= 18;
```
El SQL **no se altera**; solo se muestra:
Advertencia: `la condición parece contradictoria (rango imposible).`

## 5. Cómo probarlo desde la interfaz web

1. Compila: `make`.
2. Inicia el backend: `cd backend && python server.py`.
3. Abre `http://127.0.0.1:5050`.
4. Carga uno de los ejemplos `opt_*` (o escríbelo) y pulsa **Traducir**.
5. Aparece el panel **Optimización de código** con el SQL bruto, el SQL optimizado
   y la lista de optimizaciones y advertencias aplicadas.

## 6. Salida JSON

El ejecutable agrega estos campos (además de los existentes):

```json
{
  "sql_bruto": "...",
  "sql_optimizado": "...",
  "optimizaciones_aplicadas": [],
  "advertencias_optimizador": []
}
```
