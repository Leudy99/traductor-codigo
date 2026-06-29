# MANUAL_USUARIO.md — ConsultaLang Web Translator

## 1. Requisitos

- **MSYS2** (o Linux) con `flex`, `bison` y `gcc`.
- **Python 3** con `flask` y `flask-cors`.

Instalar dependencias de Python:
```bash
pip install flask flask-cors
```

## 2. Compilar el traductor

Desde la raíz del proyecto:
```bash
make
```
Genera el ejecutable `consultalang` (o `consultalang.exe` en Windows).

## 3. Abrir la aplicación

### Opción A — Windows (rápida)
Doble clic en `iniciar.bat`.

### Opción B — Manual
```bash
cd backend
python server.py
```
Luego abrir en el navegador: **http://127.0.0.1:5050**

## 4. Cómo escribir una consulta

En el panel **Entrada ConsultaLang**, escribe una instrucción, por ejemplo:
```txt
obtener nombre, edad desde clientes donde edad >= 18;
```

## 5. Cómo traducir

- Presiona el botón **▶ Traducir** (o `Ctrl + Enter`).
- El SQL aparece en el panel **Salida SQL**.
- El **Estado del análisis** muestra léxico, sintáctico, semántico y traducción.
- Los errores y advertencias aparecen abajo.

## 6. Usar ejemplos

En el selector **Ejemplos**, elige una opción (SELECT, INSERT, UPDATE, DELETE, CREATE TABLE) para cargarla en el editor.

## 7. Limpiar

Presiona **Limpiar** para vaciar el editor y los paneles.
