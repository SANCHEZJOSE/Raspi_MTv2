#!/bin/bash
# programa que corre el MT
#Paso 1, Compila
rm -vf prog.o
g++ -Wall -o prog.o mediciones_single.cpp ADS1115.cpp ADS1256.cpp ini/ini.c ini/cpp/INIReader.cpp -lbcm2835 -std=c++11 -pthread 
echo "Modelo compilado"
# Paso 2, Generar la fecha para el nombre de datos.txt
fecha=`date +%Y%m%d_%H:%M:%S`
echo $fecha
# Paso 3, Verificar la existencia del archivo
archivo='datos_'${fecha}'.txt'
contador=1
while [ -f "$archivo" ];
do
contador=$[contador+1]
archivo='datos_'${fecha}'('${contador}').txt'
done
# Paso 3, corre el codigo
exit
sudo ./prog.o ${archivo}  
