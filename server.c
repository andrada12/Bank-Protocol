#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <fcntl.h>
#include <netdb.h>
#include <sys/stat.h>

#define MAX_CLIENTS	5
#define BUFLEN 256


typedef struct data{

	char nume[12];
	char prenume[12];
	int numar_card;
	int pin;
	char parola_secreta[10];
	double sold;
	int nr_incercari_esuate[100];
	int in_decurs_de_deblocare;
	int socket;
}data;


typedef struct tranzactie{
	int expeditor;
	int destinatar;
	int socket_expeditor;
	double suma;
}tranzactie;

void error(char *msg)
{
    perror(msg);
    exit(1);
}

void set_to_zero(int v[]){
	int i;
	for(i = 0; i<100; i++)
		v[i] = 0;
}

void set_to_more(int v[]){
	int i;
	for(i= 0; i<100; i++)
		v[i] =4;
}

void adaugare_in_date_citite(data *d, char s[], int index){

	int j = 0, k =0;
	while(s[j] != ' ' && j < strlen(s)){
		d[index].nume[k] = s[j];
		k++; j++;
	}

	j++;
	k = 0;

	while(s[j] != ' ' && j < strlen(s)){
		d[index].prenume[k] = s[j];
		k++; j++;
	}

	j++;
	k = 0;
	char aux[20];

	while(s[j] != ' ' && j < strlen(s)){
		aux[k] = s[j];
		k++; j++;
	}	
	d[index].numar_card = atoi(aux);

	memset(aux, 0 , 20);
	j++;
	k = 0;

	while(s[j] != ' ' && j < strlen(s)){
		aux[k] = s[j];
		k++; j++;
	}	
	d[index].pin = atoi(aux);

	memset(aux, 0 , 20);
	j++;
	k = 0;

	while(s[j] != ' ' && j < strlen(s)){
		d[index].parola_secreta[k] = s[j];
		k++; j++;
	}

	j++;
	k = 0;

	while(s[j] != ' ' && j < strlen(s)){
		aux[k] = s[j];
		k++; j++;
	}
	d[index].sold = atof(aux);
	set_to_zero(d[index].nr_incercari_esuate);
	d[index].socket = 0;
	d[index].in_decurs_de_deblocare = 0;

}

void get_first_comand_arg(char s[], char *result){
	int i = 0;
	while(s[i] != ' ')
		i++;
	i++;
	int k = 0;
	while(i != ' '){
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

void stergere(int v[], int *nr_el,int elem){
	int i = 0,j;
	while(v[i] != elem)
		i++;
	for(j = i; j < (*nr_el) - 1; j++)
		v[j] = v[j + 1];
	(*nr_el) = (*nr_el) -1;

}

void stergere2(tranzactie v[], int *nr_tranz, tranzactie elem){

	int i = 0,j;
	while(v[i].destinatar != elem.destinatar && v[i].expeditor != elem.expeditor )
		i++;
	for(j = i; j < (*nr_tranz) - 1; j++){
		v[j].destinatar = v[j + 1].destinatar;
		v[j].expeditor = v[j + 1].expeditor;
		v[j].socket_expeditor = v[j + 1].socket_expeditor;
		v[j].suma = v[j + 1].suma;
	}
	(*nr_tranz) = (*nr_tranz) -1;
}

void login(char s[],int sesiuni_deschise[],int *nr_sesiuni ,data data_base[],int nr_persoane , char *raspuns, int socket){
	char aux1[20]; get_first_comand_arg(s, aux1);
	char aux2[20]; get_second_comand_arg(s, aux2);
	int numar_card = atoi(aux1);
	int pin = atoi(aux2);
	int i;
	for (i = 0; i < ( *nr_sesiuni); i++){
		if (sesiuni_deschise[i] == numar_card  ){
			strcpy(raspuns, "IBANK> −2: Sesiune  deja  deschisa\n");
			return;
		}

	}
	
	int gasit = 0;
	for (i = 0; i < nr_persoane; i++){
		if (data_base[i].numar_card == numar_card && data_base[i].nr_incercari_esuate[socket] >= 3){
			strcpy(raspuns, "IBANK> -5: Card blocat\n");
			return;
		}

		if (data_base[i].numar_card == numar_card && data_base[i].pin == pin ){
			gasit = 1;
			
			char aux[80] = "";
			strcat(aux, "IBANK> ");
			strcat(aux, "Welcome ");
			strcat(aux, data_base[i].nume);
			strcat(aux, " ");
			strcat(aux, data_base[i].prenume);
			strcat(aux, "\n");
			strcpy(raspuns, aux);
			sesiuni_deschise[(*nr_sesiuni)] = numar_card;
			(*nr_sesiuni) = (*nr_sesiuni) + 1;
			data_base[i].socket = socket;
		
		}
		
		if 	(data_base[i].numar_card == numar_card && data_base[i].pin != pin ){
			gasit = 1;
			strcpy(raspuns, "IBANK> -3: Pin gresit\n");
			data_base[i].nr_incercari_esuate[socket]++;
			if (data_base[i].nr_incercari_esuate[socket] >= 3)
				set_to_more(data_base[i].nr_incercari_esuate);

		}
	}
	if (gasit == 0)
		strcpy(raspuns, "IBANK> -4: Numar card inexistent\n");
		
}

void logout(char s[],int sesiuni_deschise[],int *nr_sesiuni ,data data_base[],int nr_persoane , char *raspuns, int socket){
	int i= 0;
	while (data_base[i].socket != socket)
		i++;
	int numar_card = data_base[i].numar_card;
	data_base[i].socket = -1;
	stergere(sesiuni_deschise, nr_sesiuni, numar_card);
	strcpy(raspuns, "IBANK> Clientul a fost deconectat\n");

}
void listsold(data data_base[],int nr_persoane,char *raspuns, int socket){
	int i = 0;
	while(data_base[i].socket != socket && i < nr_persoane)
		i++;

	if (i < nr_persoane )
		sprintf(raspuns, "%f\n", data_base[i].sold);
	else
		sprintf(raspuns, "IBANK> Clientul nu este autentificat\n");

}
void tranfer(char s[],data data_base[], int nr_persoane, char *raspuns, int socket, tranzactie tranzactii_in_asteptaare[], int *nr_tranz){
	char aux1[20]; get_first_comand_arg(s, aux1);
	char aux2[20]; get_second_comand_arg(s, aux2);
	int numar_card = atoi(aux1);
	double suma = atof(aux2);
	int card_existent = -1, client_logat = -1;
	int i;

	for(i = 0; i< nr_persoane;  i++){
		if (data_base[i].numar_card == numar_card)
			card_existent = i; 
		if (data_base[i].socket == socket)
			client_logat =data_base[i].numar_card;
	}
	if (client_logat == -1){
		strcpy(raspuns,"IABNK> Clientul nu este autentificat\n");
		return;
	}

	if (card_existent == -1){
		strcpy(raspuns, "IBANK> -4: Numar card inexistent\n");
		return;
	}
	for(i = 0; i<nr_persoane; i++){
		if (data_base[i].socket == socket && data_base[i].sold < suma){
			strcpy(raspuns, "IBANK> -8: Fonduri insuficiente\n");
			return;
		}
	}

	tranzactii_in_asteptaare [(*nr_tranz)].expeditor = client_logat;
	tranzactii_in_asteptaare [(*nr_tranz)].destinatar = numar_card;
	tranzactii_in_asteptaare [(*nr_tranz)].socket_expeditor = socket;
	tranzactii_in_asteptaare [(*nr_tranz)].suma = suma;
	(*nr_tranz) = (*nr_tranz) + 1;
	sprintf(raspuns, "IBANK> Transfer %f catre %s %s ? [y/n]\n", suma, data_base[card_existent].nume, data_base[card_existent].prenume );

}

void efectuare_transfer(char s[], data data_base[], int nr_persoane, char *raspuns, int socket, tranzactie tranzactii_in_asteptare[], int *nr_tranz){
	
	int i;
	tranzactie aux;
	for(i = 0; i<(*nr_tranz); i++){
		if (tranzactii_in_asteptare[i].socket_expeditor == socket){
			aux.expeditor = tranzactii_in_asteptare[i].expeditor;
			aux.destinatar = tranzactii_in_asteptare[i].destinatar;
			aux.socket_expeditor = tranzactii_in_asteptare[i].socket_expeditor;
			aux.suma = tranzactii_in_asteptare[i].suma;
			break;
		}
	}

	if (s[0] != 'y'){
		strcpy(raspuns, "IBANK> Operatie anulata\n");
		stergere2(tranzactii_in_asteptare, nr_tranz, aux);
		return;
	}
	
	for(i = 0; i<nr_persoane; i++){
		if (data_base[i].socket == aux.socket_expeditor)
			data_base[i].sold -= aux.suma;
		if (data_base[i].numar_card == aux.destinatar)
			data_base[i].sold += aux.suma;
	}

	strcpy(raspuns, "IBANK> Transfer realizat cu succes\n");
	stergere2(tranzactii_in_asteptare, nr_tranz, aux );

}

void unlock_pas_1(char s[], data data_base[], int nr_persoane, char *raspuns){
	int i;
	char aux1[20]; get_first_comand_arg(s, aux1);
	int numar_card = atoi(aux1);
	int gasit = -1;
	for (i = 0; i<nr_persoane; i++)
		if (data_base[i].numar_card == numar_card){
			gasit = i;
			if (data_base[i].in_decurs_de_deblocare == 1){
				strcpy(raspuns, "UNLOCK> -7: Deblocare esuata\n");
				data_base[i].in_decurs_de_deblocare = 0;
				return;
			}
			else
				data_base[i].in_decurs_de_deblocare = 1;
		}

	if (gasit != -1 && data_base[gasit].nr_incercari_esuate[1] < 3)
		strcpy(raspuns, "UNLOCK> -6: Operatie esuata\n");
	if (gasit != -1 && data_base[gasit].nr_incercari_esuate[1] >= 3)
		strcpy(raspuns, "UNLOCK> Trimite parola secreta\n");
	if (gasit == -1)
		strcpy(raspuns, "UNLOCK> -4: Numar card inexistent\n");	
}

void unlock_pas_2(char s[], data data_base[], int nr_persoane, char *raspuns){

	char aux1[20];
	strncpy(aux1, s, 6);
	char parola[20]; get_first_comand_arg(s, parola);
	int numar_card = atoi(aux1);
	int i;
	for(i = 0; i<nr_persoane; i++){
		if (data_base[i].numar_card == numar_card && strncmp(data_base[i].parola_secreta, parola, strlen(data_base[i].parola_secreta)) == 0)
			{strcpy(raspuns, "UNLOCK> Card deblocat\n");
				set_to_zero (data_base[i].nr_incercari_esuate );
				data_base[i].in_decurs_de_deblocare = 0;
			}
		if (data_base[i].numar_card == numar_card && strncmp(data_base[i].parola_secreta, parola, strlen(data_base[i].parola_secreta)) != 0){
			strcpy(raspuns, "UNLOCK> -7: Deblocare esuata\n");
			data_base[i].in_decurs_de_deblocare = 0;
		}
	}
	
}

//elibereza contul de la clientii care isi dau quit fara sa se delogheze
void elibereaza_cont(int socket,data data_base[], int nr_persoane, int sesiuni_deschise[], int *nr_sesiuni){
	int i;
	for(i = 0; i<nr_persoane; i++)
		if (data_base[i].socket == socket){
			data_base[i].socket =-1;
			int numar_card = data_base[i].numar_card;
			stergere(sesiuni_deschise, nr_sesiuni, numar_card);
		}
}



void determina_operatia(char s[], int sesiuni_deschise[],int *nr_sesiuni, data data_base[], int nr_persoane ,char *raspuns, int socket, tranzactie tranzactii_in_asteptare[], int *nr_tranz){

	int i, decide_op = 0;
	for (i = 0; i< (*nr_tranz); i++){
		if (tranzactii_in_asteptare[i].socket_expeditor == socket)
			decide_op =1;
	}
	if (decide_op == 1)
		efectuare_transfer(s, data_base, nr_persoane,raspuns,  socket, tranzactii_in_asteptare, nr_tranz);
	else{
		if (s[0] == 'l' && s[3] == 'i')
			login(s,sesiuni_deschise,nr_sesiuni ,data_base,nr_persoane,raspuns, socket);		
		if (s[0] == 'l' && s[3] == 'o')
			logout(s,sesiuni_deschise,nr_sesiuni ,data_base,nr_persoane,raspuns, socket);
		if (s[0] == 'l' && s[4] == 's')
			listsold(data_base, nr_persoane,raspuns, socket);
		if (s[0] == 't')
			tranfer(s, data_base, nr_persoane, raspuns, socket, tranzactii_in_asteptare, nr_tranz);

	}

}


int main(int argc, char *argv[])
{

	 /* Seatre variabile TCP */
	 int sockfd, newsockfd, portno, clilen;
     char buffer[BUFLEN];
     struct sockaddr_in serv_addr, cli_addr;
     int n, i, j, k;
     char s[100];
     fd_set read_fds;	
     fd_set tmp_fds;	
     int fdmax;		


     /*  Setare TCP */
     FD_ZERO(&read_fds);
     FD_ZERO(&tmp_fds);
     
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR deschidere socket");
     
     portno = atoi(argv[1]);

     memset((char *) &serv_addr, 0, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;	
     serv_addr.sin_port = htons(portno);
     
     if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr)) < 0) 
              error("ERROR binding");
     
     listen(sockfd, MAX_CLIENTS);

     
     FD_SET(0,&read_fds);
     FD_SET(sockfd, &read_fds);

     fdmax = sockfd;


     /* Setare variablie gestionare date */
     int sesiuni_deschise[100];
     data data_base[100];
     tranzactie tranzactii_in_asteptaare[100];
     int nr_tranz = 0;
     int nr_sesiuni = 0, nr_persoane =0;

     

     /* Declarare variabile UDP*/
	socklen_t addr_size;
	struct sockaddr_in server_socket;
	struct sockaddr_storage srv_str;
	server_socket.sin_port = 1025;
	
	char buf[BUFLEN];

	/*Setare UDP*/
	
	int UDPsocket = socket(PF_INET, SOCK_DGRAM, 0);
	FD_SET(UDPsocket, &read_fds);
	if (fdmax < UDPsocket)
		fdmax = UDPsocket;
	server_socket.sin_family = AF_INET;
	server_socket.sin_port = htons(1025);
	server_socket.sin_addr.s_addr = INADDR_ANY;
  	memset(server_socket.sin_zero, '\0', sizeof(server_socket.sin_zero));
	addr_size = sizeof(srv_str);

	bind(UDPsocket, (struct sockaddr*) &server_socket, sizeof(server_socket));


    if (argc < 3) {
         fprintf(stderr,"Usage : %s port\n", argv[0]);
         exit(1);
     }

     /*   CITIRE DIN FISIER   */

    FILE *f = fopen(argv[2],"r");
    if(f == NULL){ 
		printf("Nu se poate deschide la citire fişierul!\n");
		exit(1);
	}
	i = 0;
	
	while(fgets(s,100,f)){
		if (i == 0){
			nr_persoane = atoi(s);
			i++;
		} else{
			adaugare_in_date_citite(data_base, s,i-1);
			i++;
		}
	}

	for (k = 0; k<nr_persoane; k++)
		printf("%s %s %d %d %s %f\n", data_base[k].nume, data_base[k].prenume, data_base[k].numar_card, data_base[k].pin, data_base[k].parola_secreta, data_base[k].sold );

	/*	TRANSMITERE DATE*/
	 
    char raspunsUDP[30];
    
	while (1) {

		tmp_fds = read_fds; 
		if (select(fdmax + 1, &tmp_fds, NULL, NULL, NULL) == -1) 
			error("ERROR in select");


		if (FD_ISSET(0, &tmp_fds)){
					memset(buffer, 0 , BUFLEN);
					fgets(buffer, BUFLEN-1, stdin);
					if (strncmp(buffer, "quit",3) == 0){
						close(sockfd);
						close(UDPsocket);
    					return 0;
					}
				}

		if (FD_ISSET(UDPsocket, &tmp_fds) ){
					memset(buf, 0, BUFLEN);
					char raspunsUDP[80];
					recvfrom(UDPsocket, buf, BUFLEN, 0,(struct sockaddr*)&srv_str, &addr_size);
					printf("Primit in server pe UDP: %s\n",  buf);
					if (strncmp(buf, "unlock", 6) == 0)
						unlock_pas_1(buf, data_base, nr_persoane, raspunsUDP);
					else
						unlock_pas_2(buf, data_base, nr_persoane, raspunsUDP);
					
					int m = sendto(UDPsocket, raspunsUDP, strlen(raspunsUDP), 0,(struct sockaddr*)&srv_str, addr_size);
					printf("Am trimis pe UDP raspunsul: %s\n", raspunsUDP);
					if (m < 0)
						error("ERROR writing to socket UDP");
		}		
	
		for(i = 0; i <= fdmax; i++) {
			if (i != UDPsocket  && FD_ISSET(i, &tmp_fds)) {
			
				if (i == sockfd) {
					clilen = sizeof(cli_addr);
					if ((newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen)) == -1) {
						error("ERROR la accept");
					} else {
						FD_SET(newsockfd, &read_fds);
						if (newsockfd > fdmax) { 
							fdmax = newsockfd;
						}
					}
					printf("Conexiune TCP pe port %d, socket_client %d\n ", ntohs(cli_addr.sin_port), newsockfd);
				} else {
					memset(buffer, 0, BUFLEN);
					if ((n = recv(i, buffer, sizeof(buffer), 0)) <= 0) {
						if (n == 0) {
							//conexiunea s-a inchis
							elibereaza_cont(i, data_base, nr_persoane, sesiuni_deschise, &nr_sesiuni);
							printf("Server: socket %d hung up\n", i);
						} else {
							error("ERROR in recv");
						}
						close(i); 
						FD_CLR(i, &read_fds); 
					} else { 
						printf ("Am primit TCP de pe socketul %d, mesajul: %s\n", i, buffer);
						char raspuns[80];
						sprintf(raspuns, "IBANK> Nu exista comanda");
						determina_operatia(buffer, sesiuni_deschise, &nr_sesiuni, data_base, nr_persoane, raspuns, i, tranzactii_in_asteptaare, &nr_tranz);
						int m = send(i,raspuns,strlen(raspuns), 0);
						if (m < 0) 
							error("ERROR writing to socket");								
					}
				} 
			}
		}
     }


     close(sockfd);

	return 0;
}