#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/stat.h>

#define BUFLEN 256

void error(char *msg)
{
    perror(msg);
    exit(0);
}
void get_first_comand_arg(char s[], char *result){
	int i = 0;
	while(s[i] != ' ')
		i++;
	i++;
	int k = 0;
	while(s[i] != ' '){
		result[k] = s[i];
		k++;
		i++;
	}

}

void get_second_comand_arg(char s[], char *result){
	int i = 0;
	while(s[i] != ' ')
		i++;
	i++;

	while(s[i] != ' ')
		i++;
	i++;

	int k = 0;
	while(i < strlen(s)){
		result[k] = s[i];
		k++;
		i++;
	}

}

int myComp(char s[], char w[]){

	if (strlen(s) != strlen(w))
		return 0;
	int i;
	for(i = 0; i<strlen(s); i++)
		if (s[i] != w[i])
			return 0;

	return 1;	
}


int main(int argc, char *argv[])
{
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    /* Fisier de log*/
    FILE *f;
    char fname[20];
    sprintf(fname, "client-%d.log", getpid());
    f = fopen(fname, "w");


    char buffer[BUFLEN];
    if (argc < 3) {
       fprintf(stderr,"Usage %s server_address server_port\n", argv[0]);
       exit(0);
    }  
    
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));
    inet_aton(argv[1], &serv_addr.sin_addr);
    
    
    if (connect(sockfd,(struct sockaddr*) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");    
    fd_set read_fds;
    fd_set tmp_fds;
    FD_ZERO(&read_fds);
    FD_ZERO(&tmp_fds);
    FD_SET(sockfd, &read_fds);
    FD_SET(0, &read_fds);
    int i, fdmax = sockfd;
    int conectat = 0;



    /* Variabile UDP*/
	struct sockaddr_in to_station;
	socklen_t addr_size;
	char buf[BUFLEN];
	//scanf("%s", buf);


	/*Setare UDP*/
	
	int UDPsocket = socket(PF_INET, SOCK_DGRAM, 0);
	FD_SET(UDPsocket, &read_fds);
	printf("Socket UDP: %d\n", UDPsocket );
	if (fdmax < UDPsocket)
		fdmax = UDPsocket;

	to_station.sin_family = AF_INET;
	to_station.sin_port = htons(1025);
	inet_aton(argv[1], &(to_station.sin_addr));
	addr_size = sizeof(to_station);

	char last_attempt[20];
	int asteptare_parola = 0;

     
	while (1) {					
		tmp_fds = read_fds; 	
		if (select(fdmax + 1, &tmp_fds, NULL, NULL, NULL) == -1) 
			error("ERROR in select");
	
		for(i = 0; i <= fdmax; i++) {
		
			if (FD_ISSET(i, &tmp_fds) ) {
				if (i == sockfd) {
					memset(buffer, 0 , BUFLEN);
					n = recv(i, buffer, sizeof(buffer), 0);
					if (n > 0) {
						printf("%s\n", buffer);

						//scriu in fisier raspunsul severului
						fprintf(f, "%s", buffer);

						if (buffer[7] == 'W') // seteaza la conectat cand primesc mesaj welcome
							conectat = 1;
						if (buffer[7] == 'C')// seteaza la deconectat cand primesc 
							conectat = 0;
					}
					if (n == 0){
							printf("Conexiune inchisa de server\n");
							close(sockfd);
							close(UDPsocket);
							fclose(f);
							return 0;
						}
				}

				if (i == UDPsocket){
					
					memset(buffer, 0 , BUFLEN);
					int m = recvfrom(UDPsocket, buffer, BUFLEN, 0,(struct sockaddr*)&to_station, &addr_size);
					if (m < 0)
						error("ERROR recv UDP");
					printf("%s\n",  buffer);

					//scriu in fisier raspunsul serverului
					fprintf(f, "%s", buffer);

					if (asteptare_parola == 1 && buffer[8] == '-' )
						asteptare_parola = 0;
				}
	
				if(i == 0) {

						//citesc de la tastatura
						memset(buffer, 0 , BUFLEN);
						fgets(buffer, BUFLEN-1, stdin);

						//scriu in fisier comanda data
						fprintf(f, "%s", buffer);

						if (strncmp(buffer, "quit",4) == 0){
							close(sockfd);
							close(UDPsocket);
							fclose(f);
    						return 0;

						}

						if (strncmp (buffer, "login", 5) == 0) { 
							memset(last_attempt, 0 , 20);
							get_first_comand_arg(buffer, last_attempt);
							if (conectat == 1)
								{	printf("IBANK> -2: Sesiune deja deschisa\n" );
									fprintf(f, "%s", "IBANK> -2: Sesiune deja deschisa\n");
									continue;}
						}
				  		
				    	if (buffer[0] == 'l' && buffer[3] == 'o' && conectat == 0) {//verific logout
				    		printf("-1: Clientul nu este autentificat\n");
				    		fprintf(f, "%s", "-1: Clientul nu este autentificat"); 
				    	}
				    	else {
				    		if (buffer[0] == 'u' && buffer[1] == 'n'){ //aici primesc comanda de unlock
				    			asteptare_parola =1;
				    			strcpy(buffer, "unlock ");
				    			strcat(buffer, last_attempt);
				    			
				    			n = sendto(UDPsocket, buffer, strlen(buffer), 0,(struct sockaddr*)&to_station, addr_size);
				    			printf("Am trimis UDP: %s\n", buffer);
				    			
				    			if (n < 0) 
								   	 error("ERROR writing to socket");
				    		}else {
				    			if (asteptare_parola == 1){ // aici primesc parola secreta
				    				char aux3[80];
				    				strcpy(aux3, last_attempt);
				    				strcat(aux3, " ");
				    				strcat(aux3, buffer);
				    				n = sendto(UDPsocket, aux3, strlen(aux3), 0,(struct sockaddr*)&to_station, addr_size);
				    				printf("Trimis pe UDP: %s\n", aux3);
				    				asteptare_parola = 0;
				    				if (n < 0) 
								   	 error("ERROR writing to socket");		
				    			}else{	
							    	//trimit mesaj la server
							    	printf("Trimis pe TCP: %s\n", buffer );
							    	n = send(sockfd,buffer,strlen(buffer), 0);
							    	if (n < 0) 
								   	 error("ERROR writing to socket");
								}
						   }
						}
				}
			}
		}



	}


	close(sockfd);
	close(UDPsocket);
	fclose(f);

    return 0;
}
