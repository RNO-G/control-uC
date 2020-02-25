#! /bin/sh 

unzip "rno-g uC.atzip"

#for some reason everything here has the wrong time
find . -type f -exec touch {} + 
