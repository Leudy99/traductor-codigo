@echo off
REM ============================================================
REM iniciar.bat - Inicia ConsultaLang Web Translator (Windows)
REM ============================================================
echo ===========================================
echo   ConsultaLang Web Translator
echo ===========================================

REM Compilar el traductor si no existe el ejecutable
if not exist "consultalang.exe" (
    echo Compilando el traductor con make...
    make
)

echo Iniciando el servidor Flask...
echo Abrir en el navegador: http://127.0.0.1:5000
echo (Presiona Ctrl+C para detener)
cd backend
python server.py
pause
