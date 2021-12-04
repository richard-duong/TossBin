# TossBin
Toss is a unix command tool that allows a user to `toss` files or directories similar to the built-in `rm` command.
However, toss is unique in that it allows users to recover the most recently tossed version of their files / directories.

## Prerequisites
1. Must have C++ on system (necessary for g++ and make)
   a. On CentOS run: `


## Get Started
1. Navigate into TossBin
2. Run `make toss`
3. Run `chmod +x setup.sh` to give execute permissions to the setup script 
4. Run `./setup.sh` to set the toss binary to your user environment and set up crontab for automatic deletion
5. Run `toss` to get started

## How to Uninstall
1. Run `chmod +x uninstall.sh` to give execute permissions to the uninstall script
2. Run `./uninstall.sh` to remove all related components: toss binary, recycle bin, and crontab

## What can it do?
1. Toss and recover files and directories
2. Accepts absolute and relative path inputs
3. Has a temporary "~/recyclebin" directory to store tossed files
4. Maintains directory structure starting from root directory "/"
5. When recursively tossing/recovering directoriesRRR
6. Can list tossed files from recycle bin in several ways:
   1. List by most recently tossed
   2. List by name
   3. List by size
7. Force option to save time when recovering multiple existing files
8. Cron to automatically wipe older files from recycle bin after 30 days

## Future Improvements
1. Regex support
2. List recycle bin by expiration date
3. Have a way to view contents of the recycled files, e.g. --view or --see
4. Clean recyclebin directories all regular files have been removed
5. Multiple version history (similar to git version control)
6. Config file for modifying automatic recycle bin cleaning
   1. Configurable by toss date
   2. Configurable by size