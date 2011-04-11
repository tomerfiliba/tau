#!/bin/sh
rm -rf ~/soft-proj-s11/ex1
mkdir ~/soft-proj-s11/ex1
cp *.c ~/soft-proj-s11/ex1/
cp *.h ~/soft-proj-s11/ex1/
cp makefile partners.txt ~/soft-proj-s11/ex1/
chmod 755 ~
chmod 755 -R ~/soft-proj-s11
cd ~/soft-proj-s11/ex1/
make clean
make all


