#!/bin/sh

read -n 1 -p 'Will lowercase all files in current dir. Continue? [y/N] ' confirm
echo
if [[ $confirm == 'y' ]]; then
	for f in *; do
		g=`expr "$f" : '\(.*\)' | tr '[:upper:]' '[:lower:]'`
		mv -f "$f" "$g" 2> /dev/null
	done
fi
