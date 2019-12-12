#!/bin/sh
t=$(date)
u=${USER-root}
h=$(hostname)

echo '#define BUILD "' "$u@$h $t\r\n"'"' > version.h
echo '#define REV_MAJOR 0' >> version.h
echo '#define REV_MINOR 1' >> version.h
