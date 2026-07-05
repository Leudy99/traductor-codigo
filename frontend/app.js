/* ============================================================
   app.js - Logica del frontend de ConsultaLang
   ------------------------------------------------------------
   - Envia el codigo al backend (POST /translate).
   - Muestra el SQL generado y el estado de cada fase.
   - Lista errores y advertencias.
   - Maneja ejemplos, limpiar, copiar y descargar.
   ============================================================ */

// ----- Ejemplos cargables -----
const EJEMPLOS = {
    select:
        'obtener nombre, edad desde clientes donde edad >= 18;',
    completo:
        'obtener nombre, edad desde clientes donde edad >= 18 y estado = "activo" ordenar por edad descendente limite 10;',
    insert:
        'insertar en clientes nombre = "Juan", edad = 20, estado = "activo";',
    update:
        'actualizar clientes establecer estado = "inactivo" donde id = 5;',
    delete:
        'eliminar de clientes donde id = 10;',
    create:
        'crear tabla clientes {\n    id entero primario;\n    nombre texto obligatorio;\n    edad entero;\n    correo texto unico;\n}',
    opt_repetida:
        'obtener nombre desde clientes donde estado = "activo" y estado = "activo";',
    opt_or_in:
        'obtener nombre, correo desde clientes donde estado = "activo" o estado = "pendiente" o estado = "suspendido";',
    opt_contra:
        'obtener nombre desde empleados donde edad >= 65 y edad <= 18;',
};

// ----- Referencias del DOM -----
const $entrada     = document.getElementById("entrada");
const $salida      = document.getElementById("salida");
const $ejemplos    = document.getElementById("ejemplos");
const $mensajes    = document.getElementById("lista-mensajes");
const $seccionMsg  = document.getElementById("seccion-mensajes");
const $seccionOpt  = document.getElementById("seccion-optim");
const $sqlBruto    = document.getElementById("sql-bruto");
const $sqlOptim    = document.getElementById("sql-optim");
const $listaOptim  = document.getElementById("lista-optim");

const FASES = ["lexico", "sintactico", "semantico", "traduccion"];

// ----- Actualiza el panel de estado de una fase -----
function pintarEstado(fase, valor) {
    const el = document.getElementById("estado-" + fase);
    const span = el.querySelector(".valor");
    span.textContent = valor;
    el.classList.remove("ok", "error", "idle");
    if (valor === "correcto")      el.classList.add("ok");
    else if (valor === "error")    el.classList.add("error");
    else                           el.classList.add("idle");
}

// ----- Muestra errores y advertencias -----
function pintarMensajes(errores, advertencias) {
    $mensajes.innerHTML = "";
    const total = (errores?.length || 0) + (advertencias?.length || 0);
    if (total === 0) { $seccionMsg.style.display = "none"; return; }

    $seccionMsg.style.display = "block";
    (errores || []).forEach((e) => {
        const li = document.createElement("li");
        li.className = "msg-error";
        li.textContent = "❌ " + e;
        $mensajes.appendChild(li);
    });
    (advertencias || []).forEach((a) => {
        const li = document.createElement("li");
        li.className = "msg-warn";
        li.textContent = "⚠️ " + a;
        $mensajes.appendChild(li);
    });
}

// ----- Muestra la fase de optimización -----
function pintarOptim(data) {
    const aplicadas = data.optimizaciones_aplicadas || [];
    const advOpt = data.advertencias_optimizador || [];
    const bruto = data.sql_bruto || "";
    const optim = data.sql_optimizado || "";

    // Solo se muestra si hay algo que optimizar o reportar
    const hayCambio = bruto && optim && bruto !== optim;
    if (!hayCambio && aplicadas.length === 0 && advOpt.length === 0) {
        $seccionOpt.style.display = "none";
        return;
    }

    $seccionOpt.style.display = "block";
    $sqlBruto.textContent = bruto || "(sin SQL)";
    $sqlOptim.textContent = optim || "(sin SQL)";

    $listaOptim.innerHTML = "";
    aplicadas.forEach((o) => {
        const li = document.createElement("li");
        li.className = "msg-ok";
        li.textContent = "✓ " + o;
        $listaOptim.appendChild(li);
    });
    advOpt.forEach((a) => {
        const li = document.createElement("li");
        li.className = "msg-warn";
        li.textContent = "⚠️ " + a;
        $listaOptim.appendChild(li);
    });
    if (aplicadas.length === 0 && advOpt.length === 0) {
        const li = document.createElement("li");
        li.className = "msg-ok";
        li.textContent = "No se aplicaron optimizaciones (el SQL ya era óptimo).";
        $listaOptim.appendChild(li);
    }
}

// ----- Llama al backend y traduce -----
async function traducir() {
    const code = $entrada.value;
    if (!code.trim()) {
        alert("Escribe una instrucción ConsultaLang primero.");
        return;
    }

    $salida.textContent = "Traduciendo...";
    try {
        const res = await fetch("/translate", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ code }),
        });
        const data = await res.json();

        // SQL
        $salida.textContent = data.sql && data.sql.length
            ? data.sql
            : "(Sin SQL generado)";

        // Estados de cada fase
        FASES.forEach((f) => pintarEstado(f, data[f] || "—"));

        // Mensajes
        pintarMensajes(data.errores, data.advertencias);

        // Optimización
        pintarOptim(data);
    } catch (err) {
        $salida.textContent = "Error de conexión con el servidor.";
        FASES.forEach((f) => pintarEstado(f, "error"));
        pintarMensajes(["No se pudo contactar al backend: " + err.message], []);
    }
}

// ----- Limpiar todo -----
function limpiar() {
    $entrada.value = "";
    $salida.textContent = "El SQL traducido aparecerá aquí.";
    FASES.forEach((f) => pintarEstado(f, "—"));
    pintarMensajes([], []);
    $seccionOpt.style.display = "none";
    $ejemplos.value = "";
}

// ----- Eventos -----
document.getElementById("btn-traducir").addEventListener("click", traducir);
document.getElementById("btn-limpiar").addEventListener("click", limpiar);

$ejemplos.addEventListener("change", () => {
    const ej = EJEMPLOS[$ejemplos.value];
    if (ej) $entrada.value = ej;
});

// Atajo: Ctrl + Enter traduce
$entrada.addEventListener("keydown", (e) => {
    if (e.ctrlKey && e.key === "Enter") traducir();
});
