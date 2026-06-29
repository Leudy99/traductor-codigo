# ConsultaLang Web Translator

Traductor web que convierte instrucciones escritas en **español** (ConsultaLang) a **SQL estándar**, usando las fases clásicas de un compilador.

Proyecto de la asignatura de **Compiladores**.

---

## 1. Descripción

ConsultaLang es un lenguaje de consulta en español. El usuario escribe una instrucción como
`obtener nombre desde clientes donde edad >= 18;` y el sistema la traduce a SQL, mostrando además
el resultado de cada fase del análisis (léxico, sintáctico, semántico y traducción).

## 2. Objetivo

Demostrar de forma clara y educativa el flujo completo de un traductor:

```txt
Entrada → Léxico (FLEX) → Sintáctico (BISON) → Semántico (C) → Traducción → Salida SQL
```

## 3. Tecnologías

| Tecnología | Uso |
|-----------|-----|
| FLEX | Analizador léxico |
| BISON | Analizador sintáctico |
| C | Semántica, AST y generación de SQL |
| HTML / CSS / JS | Interfaz web |
| Python + Flask | Backend que conecta la web con el ejecutable en C |

## 4. Estructura del proyecto

```txt
traductor-codigo
├── frontend/   index.html, styles.css, app.js
├── backend/    server.py
├── src/        lexer.l, parser.y, ast.*, errores.*, semantica.*, generador.*, main.c
├── docs/       LENGUAJE, GRAMATICA, SEMANTICA, TRADUCCION, MANUAL_USUARIO
├── ejemplos/   select.cl, insert.cl, update.cl, delete.cl, create_table.cl
├── output/     salida.sql (generado)
├── Makefile
├── iniciar.bat
└── README.md
```

> Nota: el núcleo en C añade `ast.*` y `errores.*` para mantener separado el AST y la
> recolección de mensajes; esto hace el código más claro y fácil de explicar.

## 5. Flujo del traductor

```txt
Web (app.js) → POST /translate → Flask guarda entrada → ejecuta consultalang →
captura JSON → responde a la web → muestra SQL / estados / errores
```

## 6. Cómo compilar

Requiere `flex`, `bison`, `gcc` (disponibles en MSYS2 o Linux):

```bash
make          # genera el ejecutable consultalang
make clean    # borra los archivos generados
```

## 7. Cómo ejecutar

```bash
pip install flask flask-cors   # una sola vez
cd backend
python server.py
```

O en Windows: doble clic en **`iniciar.bat`**.

Abrir en el navegador: **http://127.0.0.1:5050**

## 8. Cómo usar la interfaz

1. Escribe una instrucción ConsultaLang (o carga un ejemplo).
2. Presiona **Traducir** (o `Ctrl + Enter`).
3. Revisa el SQL, el estado de cada fase y los mensajes.
4. Copia o descarga el SQL.

## 9. Ejemplos de entrada y salida

**Entrada**
```txt
obtener nombre, edad desde clientes donde edad >= 18 y estado = "activo" ordenar por edad descendente limite 10;
```

**Salida**
```sql
SELECT nombre, edad
FROM clientes
WHERE edad >= 18 AND estado = 'activo'
ORDER BY edad DESC
LIMIT 10;
```

## 10. Validaciones

- SELECT/INSERT/UPDATE/DELETE/CREATE sin tabla → error semántico.
- INSERT/UPDATE sin valores → error semántico.
- DELETE sin WHERE → advertencia.
- CREATE TABLE con clave primaria duplicada o columnas repetidas → error semántico.

## 11. Limitaciones

- Una instrucción por traducción.
- No valida que las tablas/columnas existan en una base de datos real (no hay catálogo).
- Subconjunto de SQL (sin JOIN, GROUP BY, subconsultas).

## 12. Conclusión

El proyecto integra las fases de un traductor (léxico, sintáctico, semántico y generación de código)
con una interfaz web moderna, cumpliendo el objetivo educativo de la asignatura de Compiladores.
