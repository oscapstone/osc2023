wc -c ./kernel8.img | sed 's/ .*//' | tr -d '\n' > $1
sleep 1
cat ./kernel8.img | pv --quiet --rate-limit 8000 > $1
