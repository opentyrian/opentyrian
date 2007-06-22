#!/bin/sh

read -n 1 -p 'Will lowercase all files in ./data directory. Continue? [y/N] ' confirm
echo
if [[ $confirm == 'y' ]]; then
	for f in data/*; do
		g=`expr "$f" : '\(.*\)' | tr '[:upper:]' '[:lower:]'`
		mv -f "$f" "$g" 2> /dev/null
	done
fi
