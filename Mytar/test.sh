#!/bin/bash
#For testing our program

ok="OK"
err="ERR"

#Check if the file exists AND executable
if [ -x mytar ]
then
	echo $ok 'The file mytar exists AND it is executable...'
else
	echo $err 'The file mytar does not exists or it is NOT executable...!!'
	exit 1;
fi

#Check if tmp exists, erase it if so
[ -d $(pwd)/tmp ] && rm -rf $(pwd)/tmp && echo $ok 'tmp dir erased...'

#Create directory tmp, switch to tmp
mkdir tmp
cd tmp
echo $ok "Changing directory to tmp..."

#Create files
echo "Hello World!" > file1.txt
head /etc/passwd > file2.txt
head -c 1024 /dev/urandom > file3.dat
echo $ok 'Files created...'

#Create tar
../mytar -c -f filetar.mtar file1.txt file2.txt file3.dat

#Create out directory
mkdir out
cp filetar.mtar out

#Swicth, run mytar extraction
cd out
../../mytar -x -f filetar.mtar

# Compare files
rm filetar.mtar

for file in $(ls)
do
	   ! diff "$file" "../$file"  > /dev/null && echo $err "The files are not equal" && cd ../../ && exit 1
done

echo $ok "Equal files..."

#Everything worked
cd ../../
echo "Success"
exit 0
