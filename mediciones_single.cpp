#include <time.h>
#include <inttypes.h>
#include <algorithm>    // std::max
#include <unistd.h>
#include <stdio.h>
#include "ADS1256.h"
#include "ADS1115.h"
#include "ini/cpp/INIReader.h"//iniciar variables con archivo de configuraciones
#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>

//using namespace std::chrono_literals;



//Variables
int clock_divisor=256;///Divisor Reloj (bcm2835 250 Mhz)
	
float vRef = 2.5,tiempo,dt,minutos_Muestro=20,save_min=5,ponderacion=0.0000625;///Volt. de referencia por defecto,Variable t(us)
int filas,columnas=6,sps=150,muestras_anteriores,blink=2,flag=0;
unsigned int time_out=10000;//tiempo en microsegundos 
float **datos;///doble puntero para matriz dinamica
struct tm  tstruct;
char       buf[80];/// fecha de guardado temporal
bool estado=false;
//funciones y macros
void setup(void);
float save_data(float **,float,float,char *);///Permite guardar los datos en un archivo .txt
float recolecta_Data(ADS1256 &,ADS1115 &,float **);///Permite recolectar los datos
///											del conversor ads1256.
struct timespec ts1, ts2;/// estructuras necesarias para calculo de 
///								tiempo en us


int main(int argc,char * argv[])
{	
	setup();
	bcm2835_init();///Inicia GPIO con la libreria bcm2835
//	FILE * archivolog=fopen(argv[1].str().c_str() +".log","w+");
//	tstruct = *localtime(&now);
//	strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
//	printf(buf);
//	printf(archivolog,"Iniciando Programa de lectura\n");
//	printf(archivolog,"Iniciando ads1116\n");
	ADS1115 adc16b;
	adc16b.begin();
	adc16b.setGain(GAIN_TWO);
	printf("ADC 16 Bits begin\n");
	ADS1256 adc24b(clock_divisor,vRef,true,2,8);///Pre config. de prot. SPI
	printf("ADC 24 Bits begin\n");
    adc24b.begin(ADS1256_DRATE_500SPS,ADS1256_GAIN_1,false);///Configuracion
	printf("Creacion Matriz dinamica\n");
	filas=sps*60.0*save_min;
	    datos = new float* [filas];///vector de punteros
    for (int i = 0; i < filas; i++){
        datos[i] = new float[columnas];///vector de 4 punteros para cada fila
    }

    printf("Iniciando lectura\n");
	    
    float min_rec = 0;
    float tbase=0;
    while (min_rec<minutos_Muestro) {

		printf("%6.2f\n",min_rec);
		dt=recolecta_Data(adc24b,adc16b,datos);
		tbase=save_data(datos,dt,tbase,argv[1]);
		printf("\tDatos temporales guardados\n");
		min_rec+=save_min;
    };  
    for (int i = 0; i <filas; i++) {
        delete[] datos[i];
    }
    delete[] datos;
//    fclose(archivolog);
return (0);

}


float recolecta_Data(ADS1256 & ads24,ADS1115 & ads16,float ** data)
{
	// programa que recolecta datos de dV para 5 canales
    int i=0;
	muestras_anteriores=0;
//	ads24.setChannel(0,1);
	clock_gettime( CLOCK_REALTIME, &ts1 );
	bool check_timeout=false;
	while(i < filas){
		flag=i;
		ads16.Differential_0_1(); 
		if ((check_timeout=ads24.waitDRDY(time_out))){//recibe como parametros en tiempo en microsegundos(aprox) para timeout
			break;
			}
		try {
			ads24.setChannel_timeout(0);
		    }
		catch(std::runtime_error& e) {
		        //std::cout << e.what() << std::endl;
		        //timedout = true;
			break;
	         }

		data[i][0]=ads24.readChannel(); // Salida de los pines AIN0 y AIN1
		data[i][4]=(float)ads16.getConversion()*ponderacion;// Salida de los pines AIN0 y AIN1 de ads1115
        ads16.Differential_2_3();
		if ((check_timeout=ads24.waitDRDY(time_out))){
			break;
			}
		ads24.setChannel_timeout(1);
		data[i][1]=ads24.readChannel(); //// Salida de los pines AIN2 y AIN3     
		data[i][5]=(float)ads16.getConversion()*ponderacion;//Salida de los pines AIN2 y AIN3 de ads1115   
		if ((check_timeout=ads24.waitDRDY(time_out))){
			break;}
		ads24.setChannel_timeout(2); 
		data[i][2]=ads24.readChannel();  // Salida de los pines AIN4 y AIN5
		if ((check_timeout=ads24.waitDRDY(time_out))){
			break;}
		ads24.setChannel_timeout(3);
		data[i][3]=ads24.readChannel();
				if(i-muestras_anteriores>=sps/blink){
		muestras_anteriores=i;
			if(estado==false){
				estado=true;}
				else{
				estado=false;}
				}
		if(estado){ bcm2835_gpio_write(12,HIGH);}
		else{ bcm2835_gpio_write(12,LOW);}
	    i++;
		}
		if(check_timeout){
			printf("TimeOUT!!\nReiniciando...");fflush(stdout);
			ads24.reboot(ADS1256_DRATE_500SPS,ADS1256_GAIN_1);///Configuracion
			printf("Retomando lectura\n");
			}
		else{
			flag=filas;}
	    clock_gettime( CLOCK_REALTIME, &ts2 );
	    tiempo=(float) ((1.0*ts2.tv_nsec - ts1.tv_nsec*1.0)*1e-9 +1.0*ts2.tv_sec - 1.0*ts1.tv_sec )*1000.0;
	    return(tiempo/(float)flag);
}
float save_data(float **matriz,float delta,float t_0,char * name){
int aux;
FILE * archivo=fopen(name,"a+");
for (int i = 0; i < flag; i++){
	     fprintf(archivo,"%.4f\t",delta*i+t_0);
	for (int j = 0; j <columnas; j++){
        fprintf(archivo,"%.9f\t",matriz[i][j]);}
        fprintf(archivo,"\n");
        aux=i+1;
        }
	fclose(archivo);
	return delta*aux+t_0;
	}
void setup(){
	    INIReader reader("config.ini");

    if (reader.ParseError() < 0) {
        printf( "Error! el archivo de config no se encuentra.\n Variables iniciadas con valores por defecto\n");

    }
    else{ minutos_Muestro = (float)reader.GetReal("configuraciones", "minutos",20.0);
           printf("TIEMPO DE MUESTREO : %f MINUTOS\n", minutos_Muestro );
           save_min = (float)reader.GetReal("configuraciones", "save",0.5);
           printf("TIEMPO PARA GUARDAR : %f MINUTOS\n", save_min );
           sps = reader.GetInteger("configuraciones", "sps",150);
           printf("MUESTRAS POR SEGUNDO %d SPS\n", sps );
           clock_divisor = reader.GetInteger("configuraciones","clock_divisor",256);
           printf("DIVISOR DEL RELOJ(BCM2835) : %d \n", clock_divisor );
           blink = reader.GetInteger("configuraciones","blink",1);
           printf("DIVISOR DE PARPADEO LED : %d \n", blink );
           printf("Variables configuradas segun 'configuraciones.ini'\n");
           }
	}
