#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

#define MAX_CONNECTIONS 15

void detect_ddos_attack();
int running = 1;
FILE *log_file;

void mejoraTime(char *timeStr) {
  timeStr[strlen(timeStr)-1]='\0'; 
}

struct ip_data {
  unsigned long ip;
  int count;
};

int find_ip(struct ip_data *ip_array, unsigned long ip) {
 for(int i = 0; i < MAX_CONNECTIONS; i++)
  if (ip_array[i].ip == ip)
     return i;
  
  return -1; //No se encontro la IP
}

void start_handler(){
  time_t currentTime = time(NULL);
  char *timeStr = ctime(&currentTime);
  mejoraTime(timeStr);
  fprintf(log_file,"[%s] Se reanudo el daemon \n",timeStr);
}

void pause_handler(){
  time_t currentTime = time(NULL);
  char *timeStr = ctime(&currentTime);
  mejoraTime(timeStr);
  fprintf(log_file,"[%s] Se pauso el daemon \n",timeStr);
  pause();
}

void stop_handler(){
  time_t currentTime = time(NULL);
  char *timeStr = ctime(&currentTime);
  mejoraTime(timeStr);
  fprintf(log_file,"[%s] Se detuvo el daemon \n",timeStr);
  running = 0;
}

static void skeleton_daemon(){
    
    char buffer[100];
    pid_t pid;

    pid = fork();

   if (pid < 0)
        exit(EXIT_FAILURE);

    if (pid > 0)
        exit(EXIT_SUCCESS);

    if (setsid() < 0)
        exit(EXIT_FAILURE);

    signal(SIGUSR1, start_handler);
    signal(SIGUSR2, pause_handler);
    signal(SIGINT,  stop_handler);

    pid = fork();
    if (pid < 0)
        exit(EXIT_FAILURE);

    if (pid > 0)
        exit(EXIT_SUCCESS);

    umask(0);

    chdir("/");

    for (int x = sysconf(_SC_OPEN_MAX); x>=0; x--) {
      close (x);
    }

   log_file = fopen ("/home/debian/Descargas/daemon/archivolog.log","a+");
   if (log_file == NULL)
      exit(EXIT_FAILURE);

   dup2(fileno(log_file),STDOUT_FILENO);
   dup2(fileno(log_file),STDERR_FILENO);

   time_t currentTime = time(NULL);
   char *timeStr = ctime(&currentTime);
   mejoraTime(timeStr);

   sprintf(buffer,"[%s]: Arranco el daemon PPID: %u \n",timeStr, getppid());
   fprintf(log_file,"%s",buffer);
	
   int iterations = 0;
   while( running ){
     iterations++;
     fprintf(log_file,"============= Iteracion %u =============\n", iterations);
     detect_ddos_attack();
     sleep(2);
    }

   fclose(log_file);
}


void  detect_ddos_attack() {
   char line[256];
   unsigned long local_adress, remote_adress;
   unsigned int local_port, remote_port;
   int state, index, elem=-1;
   
   struct ip_data ip_array[30];
   for( int i = 0; i < 30 ; i++) {
        ip_array[i].ip = 0;
        ip_array[i].count = 0;
   }

   time_t currentTime = time(NULL);
   char *timeStr = ctime(&currentTime);
   mejoraTime(timeStr);
   
   FILE *file = fopen("/proc/net/tcp", "r");
    if ( file == NULL ) {
        fprintf(log_file,"[%s] Error al abrir el archivo /proc/net/tcp \n",timeStr);
        exit(EXIT_FAILURE);
    }
    fgets(line,sizeof(line),file);
    while( fgets(line,sizeof(line),file) ){
	sscanf(line,"%*s %lx:%x %lx:%x %x", &local_adress, &local_port, &remote_adress, &remote_port, &state);
        if( remote_adress != 0 ){ // Existe un servicio escuchando la direccion
	    fprintf(log_file,"[%s] Conexion de: %lx en el puerto: %u \n", timeStr, remote_adress, local_port);
       	    index = find_ip(ip_array, remote_adress);
       	    if( index >= 0 ){
	        ip_array[index].count++;
                fprintf(log_file,"[%s] Conexion de: %lx - Cant de veces: %u \n", timeStr, remote_adress, ip_array[index].count);
                if( ip_array[index].count > MAX_CONNECTIONS )
                    fprintf(log_file,"[%s] Se debe bloquear la IP: %lx \n",timeStr,ip_array[index].ip);
            } else {
                elem++;
                ip_array[elem].ip = remote_adress;
                ip_array[elem].count = 1;
	        fprintf(log_file,"[%s] Nueva conexion de: %lx \n",timeStr, remote_adress);
            }
        }
    }
    fclose(file);
}

int main(int argc, char *argv[]) {
    int  pid;
    char nombre[50];

    if(argc != 2){
     printf("Uso: ./daemonEx <comando> \n");
     printf("El comando puede ser: -start -pause -continue -stop \n");
     exit(EXIT_FAILURE);
    }
    
    const char *comando = "ps -xj";
    FILE *fp = popen(comando, "r");
    if(fp == NULL){
       printf("Error al ejecutar el comando ps");
       exit(EXIT_FAILURE);
    }
    
    char linea[256];
    fgets(linea,sizeof(linea),fp);
    sscanf(linea,"%*s %d %*s %*s %*s %*s %*s %*s %*s %s %*s",&pid,nombre);
    while(strcmp(nombre,"./daemonEx") != 0 && !feof(fp)){
       fgets(linea,sizeof(linea),fp);
       sscanf(linea,"%*s %d %*s %*s %*s %*s %*s %*s %*s %s %*s",&pid,nombre);
    }
    if (strcmp(nombre,"./daemonEx") == 0 && pid != getpid()){ //Ya existe el demonio
      if(strcmp(argv[1],"-continue") == 0){
        kill(pid,SIGUSR1);
      } else if (strcmp(argv[1],"-pause") == 0){
          kill(pid,SIGUSR2);
        } else if (strcmp(argv[1],"-stop") == 0){
            kill(pid,SIGINT);
            } else if (strcmp(argv[1],"-start") == 0)
                printf("Comando invalido. El daemon ya se encuentra corriendo. \n");
              else
                printf("Comando no reconocido: %s \n",argv[1]);
    } else {
      if(strcmp(argv[1],"-start") == 0){
         skeleton_daemon();
      } else if (strcmp(argv[1],"-pause") == 0){ 
          printf("Comando incorrecto: %s \n",argv[1]);
          printf("Primero se debe ejecutar: ./daemonEx -start \n");
        } else if (strcmp(argv[1],"-stop") == 0){ 
            printf("Comando incorrecto: %s \n",argv[1]);
            printf("Primero se debe ejecutar: ./daemonEx -start \n");
          } else if (strcmp(argv[1],"-restart") == 0){
              printf("Comando incorrecto: %s \n",argv[1]);
              printf("Primero se debe ejecutar: ./daemonEx -start \n");
            } else  if (strcmp(argv[1],"-continue") == 0){
                printf("Comando incorrecto: %s \n",argv[1]);
                printf("Primero se debe ejecutar: ./daemonEx -start \n");
              } else
                printf("Comando no reconocido: %s \n",argv[1]);
      }
}
