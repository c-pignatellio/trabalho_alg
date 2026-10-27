CC       := gcc
STD      := -std=c11 -D_POSIX_C_SOURCE=200809L
WARN     := -Wall -Wextra -pedantic
INCLUDES := -Iinclude -Iinclude/estruturas
CFLAGS   := $(STD) $(WARN) -O2 $(INCLUDES)
LDFLAGS  :=
LDLIBS   := -lm

TARGET  := trabalho
SRC_DIR := src
OBJ_DIR := obj

# Todos os .c de src/ e src/estruturas/ (arquivos novos entram sozinhos).
SRCS := $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/estruturas/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

<<<<<<< HEAD
# Exemplo usado por "make run" e "make valgrind".
#DADOS     := dados/asteroid_dataset_20251019.csv
#CONSULTA  := consultas/nasa_q3.txt
#RESULTADO := resultado_nasa.csv
#READER    := nasa

 DADOS     := dados/deter_amazonia.csv
 CONSULTA  := consultas/deter_q1.txt
 RESULTADO := resultado_deter.csv
 READER    := deter
=======
# Dataset usado por "make run", "make valgrind" e "make todas".
# Troque aqui ou na linha de comando: make todas READER=deter ...
DADOS     := dados/asteroid_dataset_20251019.csv
CONSULTA  := consultas/nasa_q1.txt
RESULTADO := resultado_nasa.csv
READER    := nasa
# DADOS     := dados/deter_amazonia.csv
# CONSULTA  := consultas/deter_q1.txt
# RESULTADO := resultado_deter.csv
# READER    := deter
>>>>>>> 3b759c5242ede5492b0dd3e23e6064604d859c9e
ARGS    := --reader $(READER) --input $(DADOS) --base lista_enc \
           --result lista_ord --query $(CONSULTA) \
           --output $(RESULTADO) --top-k 100

# ---- Execucao de todas as combinacoes base x resultado --------------
ESTRUTURAS := pilha fila lista_seq lista_enc lista_cabeca lista_ord \
              lista_gen lista_cruz
SAIDAS     := saidas
LIMIT      := 0
TOPK       := 100
VG_LIMIT   := 5000
RUNNER     :=

.PHONY: all debug run valgrind todas valgrind-todas todos-readers clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

# -MMD -MP gera .d com as dependencias de headers: alterar um .h recompila
# apenas os .c que o incluem.
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# Depuracao: sanitizers de memoria e de comportamento indefinido.
debug: CFLAGS  := $(STD) $(WARN) -g -O0 $(INCLUDES) -fsanitize=address,undefined
debug: LDFLAGS := -fsanitize=address,undefined
debug: clean $(TARGET)

run: $(TARGET)
	./$(TARGET) $(ARGS) --limit 0

valgrind: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 \
		./$(TARGET) $(ARGS) --limit 0

# Roda as 64 combinacoes (--base x --result) com o dataset atual.
# Cada saida vai para saidas/<reader>_<base>_<result>.csv.
# Termina com erro se alguma combinacao falhar.
todas: $(TARGET)
	@mkdir -p $(SAIDAS)
	@falhas=0; total=0; \
	for b in $(ESTRUTURAS); do \
	  for r in $(ESTRUTURAS); do \
	    total=$$((total+1)); \
	    saida=$$($(RUNNER) ./$(TARGET) --reader $(READER) --input $(DADOS) \
	      --base $$b --result $$r --query $(CONSULTA) \
	      --output $(SAIDAS)/$(READER)_$${b}_$${r}.csv \
	      --top-k $(TOPK) --limit $(LIMIT) 2>&1); st=$$?; \
	    if [ $$st -eq 0 ] && echo "$$saida" | grep -q '^STATUS=OK'; then \
	      resumo=$$(echo "$$saida" | grep -E '^(MATCHED|OUTPUT)_RECORDS' | tr '\n' ' '); \
	      printf 'OK     %-6s base=%-13s result=%-13s %s\n' $(READER) $$b $$r "$$resumo"; \
	    else \
	      falhas=$$((falhas+1)); \
	      printf 'FALHOU %-6s base=%-13s result=%-13s (codigo %s)\n' $(READER) $$b $$r $$st; \
	    fi; \
	  done; \
	done; \
	echo "== $(READER): $$((total-falhas))/$$total combinacoes OK"; \
	[ $$falhas -eq 0 ]

# O mesmo, sob Valgrind (mais lento: usa --limit $(VG_LIMIT)).
valgrind-todas: $(TARGET)
	@$(MAKE) --no-print-directory todas LIMIT=$(VG_LIMIT) \
	  RUNNER="valgrind -q --leak-check=full --error-exitcode=99"

# Todas as combinacoes para os dois datasets (Parte 1 e Parte 2).
# O "-" faz o make continuar para o segundo reader mesmo se o primeiro falhar.
todos-readers: $(TARGET)
	-@$(MAKE) --no-print-directory todas READER=nasa \
	  DADOS=dados/asteroid_dataset_20251019.csv CONSULTA=consultas/nasa_q1.txt
	-@$(MAKE) --no-print-directory todas READER=deter \
	  DADOS=dados/deter_amazonia.csv CONSULTA=consultas/deter_q1.txt

clean:
	rm -rf $(OBJ_DIR) $(TARGET) $(RESULTADO) $(SAIDAS)

-include $(DEPS)