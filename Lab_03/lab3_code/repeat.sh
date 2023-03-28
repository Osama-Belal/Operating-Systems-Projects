# for i in {1..1000}
# do
#   echo "Test(" $i ") ................................................. :-" >> final_result.out
#   ./caltrain >> final_result.out
# done

start=$(date +%s.%N)

for i in {1..1000}
do
  echo "Test(" $i ") ................................................. :-" >> final_result.out
  ./caltrain >> final_result.out
done

end=$(date +%s.%N)
runtime=$(echo "$end - $start" | bc)

echo "Script completed in $runtime seconds" >> final_result.out
