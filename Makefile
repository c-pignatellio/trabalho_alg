CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -pedantic -O2 -Iinclude
LDFLAGS :=
LDLIBS  := -lm

TARGET  := trabalho
SRC_DIR := src
OBJ_DIR := obj

# Todos os .c em src/ e src/estruturas/ (novos arquivos entram sozinhos).
SRCS := $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/estruturas/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

.PHONY: all debug run valgrind clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

# -MMD -MP gera .d com as dependencias de headers: alterar um .h
# recompila apenas os .c que o incluem.
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# Versao para depuracao (sanitizers de memoria e comportamento indefinido).
debug: CFLAGS := -std=c11 -Wall -Wextra -pedantic -g -O0 -Iinclude \
                 -fsanitize=address,undefined
debug: LDFLAGS := -fsanitize=address,undefined
debug: clean $(TARGET)

# Execucao de exemplo (Parte 1).
run: $(TARGET)
	./$(TARGET) --reader nasa \
		--input dados/asteroid_dataset_20251019.csv \
		--base lista_enc --result lista_ord \
		--query consultas/q1.txt \
		--output resultado.csv --top-k 100 --limit 10000

# Verifica vazamentos de memoria (requisito: liberar tudo ao final).
valgrind: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 \
		./$(TARGET) --reader nasa \
		--input dados/asteroid_dataset_20251019.csv \
		--base lista_enc --result lista_ord \
		--query consultas/q1.txt \
		--output resultado.csv --top-k 100 --limit 1000

clean:
	rm -rf $(OBJ_DIR) $(TARGET) resultado.csv

-include $(DEPS)