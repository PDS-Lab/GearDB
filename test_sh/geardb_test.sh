#! /bin/sh

bench_db_path="/home/lzw/ceshi"
bench_value="4096"

#bench_benchmarks="fillseq,stats,readseq,readrandom,stats" #"fillrandom,fillseq,readseq,readrandom,stats"
bench_benchmarks="fillrandom,stats,readseq,readrandom,stats"
#bench_benchmarks="fillseq,stats"
bench_num="1000000"
bench_readnum="100000"
bench_read_seq_num="1000000"


const_params="
    --db=$bench_db_path \
    --value_size=$bench_value \
    --benchmarks=$bench_benchmarks \
    --num=$bench_num \
    --reads=$bench_readnum \
    --reads_seq=$bench_read_seq_num \
    "

bench_file_path="$(dirname $PWD )/out-static/db_bench"

if [ ! -f "${bench_file_path}" ];then
bench_file_path="$PWD/out-static/db_bench"
fi

if [ ! -f "${bench_file_path}" ];then
echo "Error:${bench_file_path} or $(dirname $PWD )/out-static/db_bench not find!"
exit 1
fi

cmd="$bench_file_path $const_params "

if [ -n "$1" ];then
cmd="nohup $bench_file_path $const_params >>out.out 2>&1 &"
echo $cmd >out.out
fi

echo $cmd
eval $cmd
