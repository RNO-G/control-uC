#! /bin/sh 

unzip "devboard.atzip"

#for some reason everything here has the wrong time
find . -type f -exec touch {} + 
