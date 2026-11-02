CC     := gcc
CFLAGS := -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -pedantic -O2 -Iinclude -Iinclude/estruturas
LDLIBS := -lm
TARGET := trabalho

SRCS := $(wildcard src/*.c) $(wildcard src/estruturas/*.c)
OBJS := $(SRCS:src/%.c=obj/%.o)

# Atalhos de teste (a correcao automatica chama ./trabalho direto).
# Uso: make run DATASET=deter BASE=lista_gen RES=pilha
DATASET  ?= nasa
nasa_in  := dados/asteroid_dataset_20251019.csv
nasa_q   := consultas/nasa_q1.txt
deter_in := dados/deter_amazonia.csv
deter_q  := consultas/deter_q1.txt

IN    ?= $($(DATASET)_in)
Q     ?= $($(DATASET)_q)
OUT   ?= resultado_$(DATASET).csv
BASE  ?= lista_enc
RES   ?= lista_ord
TOPK  ?= 100
LIMIT ?= 0
VG    ?=
ARGS   = --reader $(DATASET) --input $(IN) --query $(Q) --top-k $(TOPK) --limit $(LIMIT)

ESTR := pilha fila lista_seq lista_enc lista_cabeca lista_ord lista_gen lista_cruz

.PHONY: all run valgrind todas clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $^ -o $@ $(LDLIBS)

obj/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

run: $(TARGET)
	./$(TARGET) $(ARGS) --base $(BASE) --result $(RES) --output $(OUT)

valgrind: $(TARGET)
	valgrind --leak-check=full --error-exitcode=1 \
	  ./$(TARGET) $(ARGS) --base $(BASE) --result $(RES) --output $(OUT)

# Todas as combinacoes base x resultado. Ex.: make todas VG="valgrind -q --error-exitcode=99" LIMIT=5000
todas: $(TARGET)
	@mkdir -p saidas; f=0; \
	for b in $(ESTR); do for r in $(ESTR); do \
	  o=$$($(VG) ./$(TARGET) $(ARGS) --base $$b --result $$r \
	       --output saidas/$(DATASET)_$${b}_$${r}.csv 2>&1); \
	  if [ $$? -eq 0 ] && echo "$$o" | grep -q '^STATUS=OK'; then s=OK; \
	  else s=FALHOU; f=$$((f+1)); fi; \
	  printf '%-6s %-13s %-13s\n' $$s $$b $$r; \
	done; done; \
	echo "falhas: $$f/64"; [ $$f -eq 0 ]

clean:
	rm -rf obj $(TARGET) saidas resultado_*.csv

-include $(OBJS:.o=.d)