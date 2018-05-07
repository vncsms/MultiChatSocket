#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>

#define msg_limit 600

int soquet;

int rc, n, cliLen;
time_t rawtime;
struct sockaddr_in cliAddr, servAddr;


//********************estrutra cliente *********************************/

typedef struct cliente
{
    char nome[16];
    char ip[16];
    int vetorM[1000];
    struct sockaddr_in cliAddr;
} cliente;

/**********************************************************************/

cliente client[1000];

int numClientes = 0; //numero cliente


//************************* Thread que escuta os clientes ***************************/
void *escutando(void *x) 
{
    char msg[msg_limit];


    while(1)
    {

        /*Buffer*/
        memset(msg,0x0,msg_limit);
        /* RECEBENDO MENSAGEM */
        cliLen = sizeof(cliAddr);
        n = recvfrom( soquet , msg, msg_limit, 0,  (struct sockaddr *) &cliAddr, &cliLen);

	//** POST 
	//** AO RECEBER UM POST O SERVIDOR IRÁ ENVIAR ESTE POST PARA TODOS OS DEMIAS USUARIOS

        if(msg[0] == '1')
        {
            char s[522];
            strcpy(s, msg);
            char* token = strtok(s, " ");
            int aux = 0 ;
            int existe = 0;
            char nome[16];
            token = strtok(NULL, " ");
            strcpy(nome , token);
            int i = 0;
            for (i = 0 ; i < numClientes ; i++)
            {
                if(!strcmp(client[i].nome , nome ))
                {
                    existe = 1;
                    break;
                }
            }
            int h = i;
            token = strtok(NULL, " ");
            char mandar[600];
            strcpy(mandar , "");

  	    time (&rawtime);
            strcat(mandar , ctime (&rawtime));

            if(existe)
            {
                printf("%s :: " , nome);
                strcat(mandar,nome);
                strcat(mandar," :: ");

                while (token)
                {

                    printf("%s " , token);
                    strcat(mandar,token);
                    strcat(mandar," ");
                    token = strtok(NULL, " ");

                }
                printf("\n");

                for (i = 0 ; i < numClientes ; i++)
                {
                    if(h!=i) // NAO ENVIAR UM POST PARA A PESSOA QUE ENVIOU O POST
                    {
                        if(client[i].vetorM[h]==0)//SE O USUARIO NAO ESTAH MUTADO ENTAO RECEBE MENSAGEM
                        {
			    time (&rawtime);
                            printf("%s: Mandando de %s para %s\n" ,ctime (&rawtime) , client[h].nome , client[i].nome );
                            sendto( soquet ,mandar,sizeof(mandar),0,(struct sockaddr *) &client[i].cliAddr,sizeof(client[i].cliAddr));
                        }
                        else // CASO CONTRARIO ELE NAO RECEBERÁ MENSAGEM
                        {
			    time (&rawtime);
                            printf("%s: %s estah mutato para %s\n" , ctime (&rawtime) ,client[i].nome , client[h].nome);
                        }
                    }
                }
            }
        }

	//** AO RECEBER UM MENSAGEM DE CONECTAR O SERVIDOR IRÁ VERIFICAR SE O NOME DE USUARIO JÁ ESTAH SENDO USADO
        else if(msg[0] == '2') // conectar
        {
            int i = 0;

            char s[522];
            strcpy(s, msg);
            char* token = strtok(s, " ");
            int aux = 0 ;
            int existe = 0;
            char nome[16];
            char ip[16];

            //verifica se existe alguem já com o nick
            while (token)
            {
                if(aux == 1 )
                {
                    strcpy(nome , token);
                }
                if(aux == 2 )
                {
                    strcpy(ip , token);
                    break;
                }
                token = strtok(NULL, " ");
                aux++;
            }

            for(i = 0 ; i < numClientes ; i++)
            {
                if(!strcmp(client[i].nome , nome) || !strcmp(client[i].ip , ip))
                {
                    existe = 1;
                    break;
                }
            }

            if(!existe)//SE O NOME DE USUARIO OU IP NAO FOR USADO ENTAO  ELE E CADASTRADO
            {
                strcpy(client[numClientes].nome , nome);
                strcpy(client[numClientes].ip , ip);
                client[numClientes].cliAddr = cliAddr;
		time (&rawtime);

		char mandar[600];
		strcpy(mandar , "");
		for (i = 0 ; i < numClientes ; i++)
                {
               	    time (&rawtime);
		    strcat(mandar , ctime (&rawtime));
		    strcat(mandar , ": USUARIO: ");
		    strcat(mandar , nome );
                    strcat(mandar , " entrou na sala\n");
                    sendto( soquet ,mandar,sizeof(mandar)+1,0,(struct sockaddr *) &client[i].cliAddr,sizeof(client[i].cliAddr));
                     
                }

                printf("%s: USUARIO: %s entrou na sala %s\n" , ctime (&rawtime) , nome , ip);
                sendto( soquet ,"1",sizeof("1"),0,(struct sockaddr *) &cliAddr,sizeof(cliAddr));
                numClientes++;

                for(i = 0 ; i < 1000 ; i++)
                {
                    client[numClientes-1].vetorM[i] = 0;
                }
                for(i = 0 ; i <numClientes ; i++)
                {
                    client[i].vetorM[numClientes-1] = 0;
                }
            }
            else//SE HOUVER ALGUEM COM O MESMO IP OU NOME DE USUARIO ENTAO CANCELA A INSCRICAO
            {

		time (&rawtime);
                printf("%s: Cliente já existente\n" , ctime (&rawtime));
                sendto( soquet ,"0",sizeof("0"),0,(struct sockaddr *) &cliAddr,sizeof(cliAddr));
            }

        }
        else if(msg[0] == '3') // MUTE
        {
            int i = 0;
            int j = 0 ;
            char s[522];
            strcpy(s, msg);
            char* token = strtok(s, " ");
            int aux = 0 ;
            int existe = 0;
            char alvo[16];
            char origem[16];
            token = strtok(NULL, " ");
            strcpy(origem , token);
            for(i = 0 ; i < numClientes ; i++)
            {
                if(!strcmp(client[i].nome , origem))
                {
                    token = strtok(NULL, " ");
                    while (token)
                    {
                        existe = 0;
                        for(j= 0 ; j < numClientes ; j++)
                        {
                            if(!strcmp(client[j].nome , token))
                            {
                                client[i].vetorM[j] = 1;
                                existe = 1;
                                break;
                            }
                        }
                        if(existe)
                        {
			    time (&rawtime);
                            printf("%s: %s mutou %s com sucesso \n" ,ctime (&rawtime) , origem , token);
                        }
                        else
                        {
			    time (&rawtime);
                            printf("%s: %s nao mutou %s , pois ele nao existe \n"  , ctime (&rawtime), origem , token);
                        }
                        token = strtok(NULL, " ");
                        aux++;
                    }
                    break;
                }
            }


        }
	
	// VERIFICA O CLOSE, SUBTRAI O USUARIO DA LISTA DE CONECTADOS
        else if(msg[0] == '4')
        {
            int i = 0;
            int j = 0 ;
            char s[522];
            strcpy(s, msg);
            char* token = strtok(s, " ");
            int aux = 0 ;
            int existe = 0;
            char nome[16];
            token = strtok(NULL, " ");
            strcpy(nome , token);

	    

            for(i = 0 ; i < numClientes ; i++)
            {
                if(!strcmp(client[i].nome , nome))
                {
                    existe= 1;
                    break;
                }
            }
            if(existe)
            {

                int posdel = i;
                if(posdel == numClientes-1)
                {
                    numClientes --;
                }
                else
                {
                    for(i = posdel ; i <  numClientes-1; i++)
                    {
                        client[i] = client[i+1];
                    }
                    numClientes --;
                    for(i = 0 ; i < numClientes ; i++)
                    {
                        for(j = 0  ; j < numClientes ; j++)
                        {
                            client[i].vetorM[j] = client[i].vetorM[j+1];
                        }
                    }
                }

		char mandar[600];
		strcpy(mandar , "");
		for (i = 0 ; i < numClientes ; i++)
                {
               	    time (&rawtime);
		    strcat(mandar , ctime (&rawtime));
		    strcat(mandar , ": USUARIO: ");
		    strcat(mandar , nome );
                    strcat(mandar , " saiu da sala\n");
                    sendto( soquet ,mandar,sizeof(mandar)+1,0,(struct sockaddr *) &client[i].cliAddr,sizeof(client[i].cliAddr));
                     
                }

		time (&rawtime);
                printf("%s: usuario: %s saiu do chat \n", ctime (&rawtime) , nome);
            }
            else
            {
		time (&rawtime);
                printf("%s: usuario: %s deletado com falha \n" , ctime (&rawtime),  nome);
            }
        }
        else if(msg[0] == '5') //unmute
        {
            int i = 0;
            int j = 0 ;
            char s[522];
            strcpy(s, msg);
            char* token = strtok(s, " ");
            int aux = 0 ;
            int existe = 0;
            char alvo[16];
            char origem[16];
            token = strtok(NULL, " ");
            strcpy(origem , token);
            for(i = 0 ; i < numClientes ; i++)
            {
                if(!strcmp(client[i].nome , origem))
                {
                    token = strtok(NULL, " ");
                    while (token)
                    {
                        existe = 0;
                        for(j= 0 ; j < numClientes ; j++)
                        {
                            if(!strcmp(client[j].nome , token))
                            {
                                client[i].vetorM[j] = 0;
                                existe = 1;
                                break;
                            }
                        }
                        if(existe)
                        {
			    time (&rawtime);
                            printf("%s: %s desmutou %s com sucesso \n" ,  ctime (&rawtime),  origem , token);
                        }
                        else
                        {
			    time (&rawtime);
                            printf("%s: %s nao desmutou %s , pois ele nao existe \n" ,  ctime (&rawtime), origem , token);
                        }
                        token = strtok(NULL, " ");
                        aux++;
                    }
                    break;
                }
            }
        }

        if(n<0)
        {
            printf("ERRO AO RECEBER MENSAGEM \n");
            continue;
        }
    }
}

int main(int argc, char *argv[])
{


    char msg[msg_limit];
    pthread_t thread;

    //verificando o numero de argumentos
    if(argc!= 2)
    {
        printf("Erro na entrada\n");
        printf("%s <porta>\n", argv[0]);
        return 0;
    }

    //definindo a porta
    int porta  = atoi(argv[1]);


    /* socket creation */
    soquet =socket(AF_INET, SOCK_DGRAM, 0);
    if (soquet <0)
    {
        printf("erro ao criar soquete \n");
        return 0;
    }

    // INICIALIZANDO SERVADDR E BINDANDO A PORTA 
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(porta);

    rc = bind(  soquet , (struct sockaddr *) &servAddr,sizeof(servAddr));

    if(rc<0)
    {
        printf("Erro ao conectar a porta %d \n", porta);
        return 0;
    }

    int x;

    /*LANÇANDO A THREAD*/
    if(pthread_create(&thread, NULL, escutando , &x))
    {

        printf("ERRO AO CRIAR THREAD\n");
        return 1;

    }


    /*LAÇO PRINCIPAL*/
    while(0)
    {
        /*char entrada[msg_limit];
        scanf("%s" , entrada);
        printf("%s" , entrada);
        sendto( soquet ,"1",sizeof("1"),0,(struct sockaddr *) &cliAddr,sizeof(cliAddr));*/
    }

    /*esperando terminar a thread*/
    if(pthread_join(thread, NULL))
    {

        printf("Erro ao finalizar a thread\n");
        return 2;

    }
    return 0;

}

