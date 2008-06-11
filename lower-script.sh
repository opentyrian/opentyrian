#!/bin/bash

if [[ $1 ]]; then
	target=$1
else
	target="./data"
fi

read -p "Lowercase all files in $target directory? [y/N] " confirm

if [[ `echo $confirm | tr '[:upper:]' '[:lower:]'` == 'y' ]]; then
	for f in $target/*; do
		g=`basename "$f" | tr '[:upper:]' '[:lower:]'`
		mv -fv "$f" "$target/$g" 2> /dev/null
	done
fi
