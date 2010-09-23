#!/bin/bash
if [ ! -d ./content/proc ]; then
	mkdir ./content/proc
	mkdir ./content/sys
	tar -xzvpf dev.tar.gz -C ./content/
fi
