# ============================================================
# Makefile - Compilacion del traductor ConsultaLang
# ------------------------------------------------------------
#   make          -> genera el ejecutable 'consultalang'
#   make clean    -> borra archivos generados
#
# Requiere: flex, bison, gcc (disponibles en MSYS2 / Linux).
# ============================================================

CC      = gcc
CFLAGS  = -Wall -O2
SRC     = src
BIN     = consultalang

# Fuentes en C del proyecto (sin contar los generados)
FUENTES = $(SRC)/ast.c $(SRC)/errores.c $(SRC)/semantica.c \
          $(SRC)/generador.c $(SRC)/optimizador.c \
          $(SRC)/generador_destino.c $(SRC)/main.c

# Archivos generados por bison/flex
GEN     = $(SRC)/parser.tab.c $(SRC)/parser.tab.h $(SRC)/lex.yy.c

all: $(BIN)

# Analizador sintactico (BISON) -> parser.tab.c + parser.tab.h
$(SRC)/parser.tab.c $(SRC)/parser.tab.h: $(SRC)/parser.y
	bison -d -o $(SRC)/parser.tab.c $(SRC)/parser.y

# Analizador lexico (FLEX) -> lex.yy.c
$(SRC)/lex.yy.c: $(SRC)/lexer.l $(SRC)/parser.tab.h
	flex -o $(SRC)/lex.yy.c $(SRC)/lexer.l

# Enlazado final
$(BIN): $(SRC)/parser.tab.c $(SRC)/lex.yy.c $(FUENTES)
	$(CC) $(CFLAGS) -I$(SRC) -o $(BIN) \
	    $(SRC)/parser.tab.c $(SRC)/lex.yy.c $(FUENTES)

clean:
	rm -f $(GEN) $(BIN) $(BIN).exe

.PHONY: all clean
