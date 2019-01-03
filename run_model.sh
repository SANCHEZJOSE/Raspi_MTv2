#!/bin/bash
# programa que corre el MT
#paso 1 compila
rm -v prog.o
g++ --std=c++0x -Wall -o prog.o mediciones.cpp ADS1115.cpp ADS1256.cpp ini/ini.c ini/cpp/INIReader.cpp -lbcm2835
echo "Modelo compilado"
# Paso 2, generar el nombre de datos.txt
fecha=`date +%Y%m%d_%H:%M:%S`
echo $fecha
# Paso 3, corre el codigo
#sudo ./prog.o datos${fecha}.txt
# TEST 
