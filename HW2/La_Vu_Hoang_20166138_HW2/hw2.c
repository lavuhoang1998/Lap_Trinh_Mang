#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
int main(int argc, char *argv[]){
	int i;
	struct hostent *he;
	struct in_addr **addr_list;
	struct in_addr ipv4addr;
	if(argc !=2){
		printf("Sai so luong doi so truyen vao. Nhap lai theo cu phap ./resolver B\n");
	}
	else{
		char xaunhapvao[100];
		strcpy(xaunhapvao,argv[1]);
		if(inet_addr(xaunhapvao)==-1){
			he = gethostbyname(xaunhapvao);
			if(he!=NULL){
				addr_list = (struct in_addr **)he->h_addr_list;
				printf("Official IP: %s\n",inet_ntoa(*addr_list[0]));
    			printf("Alias IP:\n");
    			for(i = 1; addr_list[i] != NULL; i++) {
        			printf("%s \n", inet_ntoa(*addr_list[i]));     //chuyen cau truc in_addr sang cau truc a.b.c.d
    			}

			} 
			else printf("Not found information\n");


		}
		else{
			inet_pton(AF_INET, xaunhapvao, &ipv4addr);
			he = gethostbyaddr(&ipv4addr, sizeof ipv4addr, AF_INET);
			if(he!=NULL){
			printf("Official name: %s\n", he->h_name);
			printf("Alias name:\n");
			while(*he->h_aliases){
					printf("%s\n", *he->h_aliases);
					he->h_aliases++;
				
				}
			}
			else printf("Not found information\n");

		}


	}

	return 0;
}