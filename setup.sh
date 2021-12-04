#!/bin/bash

# setup toss
chmod +x bin/toss
sudo cp bin/toss /usr/local/bin
mkdir ~/.recyclebin

# setup cron job for automatic file deletion
croncmd="find ~/recyclebin -mtime +30 -delete;"
cronjob="0 0 * * * $croncmd"
( crontab -l | grep -v -F "$croncmd" ; echo "$cronjob" ) | crontab -