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

# Exemplo usado por "make run" e "make valgrind".
DADOS     := dados/asteroid_dataset_20251019.csv
CONSULTA  := consultas/nasa_q1.txt
RESULTADO := resultado_nasa.csv
READER    := nasa
# DADOS     := dados/deter_amazonia.csv
# CONSULTA  := consultas/deter_q1.txt
# RESULTADO := resultado_deter.csv
# READER    := deter
ARGS    := --reader $(READER) --input $(DADOS) --base lista_enc \
           --result lista_ord --query $(CONSULTA) \
           --output $(RESULTADO) --top-k 100

.PHONY: all debug run valgrind clean

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

clean:
	rm -rf $(OBJ_DIR) $(TARGET) $(RESULTADO)

-include $(DEPS)