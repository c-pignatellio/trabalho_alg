# ------- configuração do experimento (troque AQUI) -------
REP=3
DATASET="dados/asteroid_dataset_20251019.csv"
QUERY="consultas/nasa_q1.txt"
READER="nasa"
BASE="lista_enc"
RESULT="lista_ord"
OUT_DIR="bench/${READER}_${BASE}_${RESULT}"
BIN="./trabalho"

N_LIST=(1000 5000 10000 25000 50000 100000)
K_LIST=(10 50 100 500 0)   # 0 = todos
# --------------------------------------------------------

: > "$OUT_DIR/raw.csv"        # trunca se existir
echo "N,K,rep,INPUT,MATCHED,OUTPUT,LOAD_MS,SEARCH_MS,RESULT_INSERT_MS,WRITE_MS" \
  > "$OUT_DIR/tempos.csv"

for N in "${N_LIST[@]}"; do
  for K in "${K_LIST[@]}"; do
    for r in $(seq 1 "$REP"); do
      tmp_out="$OUT_DIR/tmp_${N}_${K}_${r}.csv"

      stdout=$("$BIN" \
        --reader "$READER" --input "$DATASET" \
        --base "$BASE" --result "$RESULT" \
        --query "$QUERY" --output "$tmp_out" \
        --top-k "$K" --limit "$N" 2>&1) ; status=$?

      if [ $status -ne 0 ] || ! grep -q '^STATUS=OK' <<<"$stdout"; then
        printf 'FALHOU N=%s K=%s r=%s status=%s\n' "$N" "$K" "$r" "$status" >&2
        echo "----- stdout -----" >&2
        echo "$stdout" >&2
        echo "------------------" >&2
        exit 1
      fi

      linha=$(awk -v n="$N" -v k="$K" -v r="$r" '
        /^INPUT_RECORDS=/    {split($0,a,"="); in_=a[2]}
        /^MATCHED_RECORDS=/  {split($0,a,"="); mat=a[2]}
        /^OUTPUT_RECORDS=/   {split($0,a,"="); out_=a[2]}
        /^LOAD_MS=/          {split($0,a,"="); load=a[2]}
        /^SEARCH_MS=/        {split($0,a,"="); search=a[2]}
        /^RESULT_INSERT_MS=/ {split($0,a,"="); rins=a[2]}
        /^WRITE_MS=/         {split($0,a,"="); w=a[2]}
        END { printf "%s,%s,%s,%d,%d,%d,%s,%s,%s,%s\n",
                n,k,r, in_,mat,out_, load,search,rins,w }' <<<"$stdout")

      echo "$linha" >> "$OUT_DIR/tempos.csv"
      rm -f "$tmp_out"
    done
  done
done

total_exec=$(( ${#N_LIST[@]} * ${#K_LIST[@]} * REP ))
echo "OK: ${total_exec} execucoes. CSV em ${OUT_DIR}/tempos.csv"