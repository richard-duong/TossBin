#!/bin/bash
# My failure
# find ~ -type f -exec stat -c '%Z' {} + | awk '{print $1}' > dates.txt
# find ~ -type f -exec stat -c '%n' {} + | awk '{print $1}' > filenames.txt

# ls ~ -Rltc --time-style='+%Y-%m-%d %H:%M:%S' | awk '{print $6,$7}' > dates.txt

# ls ~ -Rltc --time-style='+%Y-%m-%d %H:%M:%S' | awk '{print $6,$7}'
# today_date=$(date +"%Y-%m-%d %H:%M:%S")
# echo $today_date


# ls -Rltc 

# for d in $(cat dates.txt); do 
    # echo "$(($(date -d "$today_date" '+%d') - $(date -d "$d" '+%d')))" 
# done

    
# date -u -d @$(($(date -d "$Value2" '+%s') - $(date -d "$Value1" '+%s'))) '+%T'

# -------------------------- NEW ONE LINER ----------------------------

# Finds files that are modifed older than 30 days ago
find ~/recyclebin -mtime +30
find /tmp -mtime +30

