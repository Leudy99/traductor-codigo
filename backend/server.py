# ============================================================
# server.py - Backend Flask de ConsultaLang Web Translator
# ------------------------------------------------------------
# Conecta la interfaz web con el traductor compilado en C.
# Flujo:
#   1. Recibe el codigo ConsultaLang por POST /translate.
#   2. Lo guarda en un archivo temporal (sin BOM).
#   3. Ejecuta el ejecutable 'consultalang'.
#   4. Captura el JSON que imprime el traductor.
#   5. Lo devuelve al frontend.
# ============================================================
import json
import os
import subprocess

from flask import Flask, jsonify, request, send_from_directory
from flask_cors import CORS

# ----- Rutas del proyecto -----
BASE_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
FRONTEND_DIR = os.path.join(BASE_DIR, "frontend")
OUTPUT_DIR = os.path.join(BASE_DIR, "output")
TEMP_INPUT = os.path.join(OUTPUT_DIR, "entrada_temp.cl")

# Nombre del ejecutable segun el sistema operativo
EXE_NAME = "consultalang.exe" if os.name == "nt" else "consultalang"
EXE_PATH = os.path.join(BASE_DIR, EXE_NAME)

app = Flask(__name__, static_folder=FRONTEND_DIR, static_url_path="")
CORS(app)


def respuesta_error(mensaje):
    """Construye una respuesta JSON de error con la forma esperada."""
    return {
        "success": False,
        "sql": "",
        "sql_bruto": "",
        "sql_optimizado": "",
        "lexico": "no ejecutado",
        "sintactico": "no ejecutado",
        "semantico": "no ejecutado",
        "traduccion": "no ejecutado",
        "errores": [mensaje],
        "advertencias": [],
        "optimizaciones_aplicadas": [],
        "advertencias_optimizador": [],
    }


@app.route("/")
def index():
    """Sirve la pagina principal."""
    return send_from_directory(FRONTEND_DIR, "index.html")


@app.route("/<path:archivo>")
def estaticos(archivo):
    """Sirve archivos estaticos del frontend (css, js)."""
    return send_from_directory(FRONTEND_DIR, archivo)


@app.route("/translate", methods=["POST"])
def translate():
    """Traduce codigo ConsultaLang a SQL usando el ejecutable en C."""
    datos = request.get_json(silent=True) or {}
    codigo = datos.get("code", "")

    if not codigo.strip():
        return jsonify(respuesta_error("No se recibio codigo para traducir."))

    if not os.path.exists(EXE_PATH):
        return jsonify(respuesta_error(
            "El traductor no esta compilado. Ejecuta 'make' primero."))

    # 1. Guardar la entrada en un archivo temporal sin BOM
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    with open(TEMP_INPUT, "w", encoding="utf-8") as f:
        f.write(codigo)

    # 2. Ejecutar el traductor (cwd = raiz para que escriba output/salida.sql)
    try:
        proc = subprocess.run(
            [EXE_PATH, TEMP_INPUT],
            capture_output=True,
            text=True,
            encoding="utf-8",
            cwd=BASE_DIR,
            timeout=10,
        )
    except subprocess.TimeoutExpired:
        return jsonify(respuesta_error("El traductor tardo demasiado (timeout)."))
    except OSError as e:
        return jsonify(respuesta_error(f"No se pudo ejecutar el traductor: {e}"))

    # 3. Interpretar la salida JSON del traductor
    salida = (proc.stdout or "").strip()
    try:
        resultado = json.loads(salida)
    except json.JSONDecodeError:
        return jsonify(respuesta_error(
            "Salida invalida del traductor: " + (salida or proc.stderr or "vacia")))

    return jsonify(resultado)


if __name__ == "__main__":
    PORT = 5050
    print("ConsultaLang Web Translator")
    print(f"Abrir en el navegador: http://127.0.0.1:{PORT}")
    app.run(host="127.0.0.1", port=PORT, debug=True)
