# Name: Omar Radwan
# UID: 205105562
# Class: CS118 Spring 2020
# Project: Project 1

default:
	gcc -g -Wall -Werror webserver.c -o webserver
build: default
	tar -cf 205105562.tar.gz webserver.c Makefile 404.html index.html words.txt example.png paris.jpg
dist: default
	tar -cf 205105562.tar.gz webserver.c Makefile 404.html index.html words.txt example.png paris.jpg
clean: build default
	-rm webserver *tar.gz
