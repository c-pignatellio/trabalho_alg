#!/usr/bin/env bash
# ------- configuração do experimento (troque AQUI) -------
set -euo pipefail

REP=3
READER="nasa"
DATASET="dados/asteroid_dataset_20251019.csv"
QUERY="consultas/nasa_q1.txt"
BIN="./trabalho"

MODE="${MODE:-estruturas}"

case "$MODE" in
  escala)
    N_LIST=(1000 5000 10000 25000 50000 100000)
    K_LIST=(10 50 100 500 0)
    BASE_LIST=(lista_enc)
    RESULT_LIST=(lista_ord)
    ;;
  estruturas)
    N_LIST=(50000)
    K_LIST=(10 100 0)
    BASE_LIST=(pilha fila lista_seq lista_enc lista_cabeca lista_ord lista_gen lista_cruz)
    RESULT_LIST=(pilha fila lista_seq lista_enc lista_cabeca lista_ord lista_gen lista_cruz)
    ;;
  *) echo "MODE invalido: $MODE" >&2; exit 1 ;;
esac

OUT_DIR="bench/${MODE}_${READER}"
mkdir -p "$OUT_DIR"
RAW="$OUT_DIR/raw.csv"
: > "$RAW"
echo "N,K,rep,base,result,INPUT,MATCHED,OUTPUT,LOAD_MS,SEARCH_MS,RESULT_INSERT_MS,WRITE_MS" > "$RAW"

trap 'rm -f "$OUT_DIR"/tmp_*.csv' EXIT

for B in "${BASE_LIST[@]}"; do
  for R in "${RESULT_LIST[@]}"; do
    for N in "${N_LIST[@]}"; do
      for K in "${K_LIST[@]}"; do
        for r in $(seq 1 "$REP"); do
          tmp="$OUT_DIR/tmp_${B}_${R}_${N}_${K}_${r}.csv"
          if stdout=$("$BIN" \
                --reader "$READER" --input "$DATASET" \
                --base "$B" --result "$R" \
                --query "$QUERY" --output "$tmp" \
                --top-k "$K" --limit "$N" 2>&1) \
             && grep -q '^STATUS=OK' <<<"$stdout"; then
            linha=$(awk -v n="$N" -v k="$K" -v r="$r" -v b="$B" -v res="$R" '
              /^INPUT_RECORDS=/    {split($0,a,"="); in_=a[2]}
              /^MATCHED_RECORDS=/  {split($0,a,"="); mat=a[2]}
              /^OUTPUT_RECORDS=/   {split($0,a,"="); out_=a[2]}
              /^LOAD_MS=/          {split($0,a,"="); load=a[2]}
              /^SEARCH_MS=/        {split($0,a,"="); search=a[2]}
              /^RESULT_INSERT_MS=/ {split($0,a,"="); rins=a[2]}
              /^WRITE_MS=/         {split($0,a,"="); w=a[2]}
              END { printf "%s,%s,%s,%s,%s,%d,%d,%d,%s,%s,%s,%s\n",
                      n,k,r,b,res, in_,mat,out_, load,search,rins,w }' <<<"$stdout")
            echo "$linha" >> "$RAW"
            rm -f "$tmp"
          else
            echo "FALHOU B=$B R=$R N=$N K=$K r=$r" >&2
            echo "$stdout" >&2
            rm -f "$tmp"
          fi
        done
      done
    done
  done
done

total_exec=$(( ${#N_LIST[@]} * ${#K_LIST[@]} * ${#BASE_LIST[@]} * ${#RESULT_LIST[@]} * REP ))
echo "OK: ${total_exec} execucoes. CSV em $RAW"