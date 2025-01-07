#!/bin/bash

file='config.txt'

shopt -s extglob

while IFS='=' read -r key value
do 
    # remove whitespaces
    key="${key%%*( )}"
    key="${key##*( )}"
    if [ "$key" == "MODULE_TYPE" ]; then
        # remove whitespaces  
        value="${value%%*( )}"
        value="${value##*( )}"
        echo $key
        echo $value
     fi
done < "$file"

shopt -u extglob

