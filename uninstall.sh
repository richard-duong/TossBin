#!/bin/bash

# uninstall toss
sudo rm /usr/local/bin/toss
sudo rm -r ~/recyclebin

# remove cron job
croncmd="find ~/recyclebin -mtime +30 -delete; find /tmp -mtime +30 -delete"
cronjob="0 0 * * * $croncmd"
( crontab -l | grep -v -F "$croncmd" ) | crontab -