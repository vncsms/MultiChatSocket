#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

#define msg_limit 600

int soquet,conexao, i;
struct sockaddr_in cliente_addr, servidor_addr;
struct hostent *h;


//função da thread que realiza a escuta das mensagens
void *escutando(void *x)
{
    char msg[msg_limit];
    while(1)
    {
        int servlen = sizeof(servidor_addr);
        recvfrom( soquet , msg, msg_limit, 0,  (struct sockaddr *) &servidor_addr, &servlen);
        printf("%s \n" , msg);
    }
}


int main(int argc, char *argv[])
{


    int porta = 0;      //porta de acesso
    char nick[16];	//nick unico do usuario
    char comando[522];  //comando do usuario
    char s[522];
    char msg[600]; 	//mensagem a ser enviada
    pthread_t thread;   //thread para "parelizar"

    //**************************verificando o numero de argumentos**************************//
    //* 1: ip do usuarop   2: porta a ser acessada  3: nome de usuario
    //* Só é aceito se o usuario enviar exatos 4 argumentos 
    //*Não é verificado se os arqumentos estão certos, apenas a quantidade de argumentos

    if(argc!= 4)
    {
        printf("Erro na entrada\n");
        printf("%s <ip/nome><porta><usuario>\n", argv[0]);
        return 0;
    }

    /****************************************************************************************/

    //*************************definindo a porta e o nick usuario***************************//
    //*O nick inicial e final é decidido pela ultima entrada do programa 

    porta  = atoi(argv[2]);
    strcpy(nick , argv[3]);
    h = gethostbyname(argv[1]);

    /*Caso ocorrer um erro nos host*/

    if(h==NULL)
    {
        printf("host desconhecido '%s' \n", argv[1]);
        return 0;
    }

    
    /******************************* INICIZLIZANDO SERVIDOR_ADDR ****************************/

    servidor_addr.sin_family = h->h_addrtype;
    memcpy((char *) &servidor_addr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
    servidor_addr.sin_port = htons(porta);

    /****************************************************************************************/

    //*************************** CRIANDO O SOQUETE *****************************************/

    soquet = socket(AF_INET,SOCK_DGRAM,0);

    /*Erro no soquete*/

    if(soquet<0)
    {
        printf("Tem erro no soquete \n");
        return 0;
    }

    /*****************************************************************************************/

    //***************************** bindando a porta, conectaando ****************************/

    cliente_addr.sin_family = AF_INET;
    cliente_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    cliente_addr.sin_port = htons(0);
    conexao = bind(soquet, (struct sockaddr *) &cliente_addr, sizeof(cliente_addr));

    /*Erro na conexao*/
    if(conexao<0)
    {
        printf("Nao foi possivel bindar a porta");
        return 0;
    }

    /*****************************************************************************************/

    /***************************** realizando um cadastro no servidor ************************/
    //** Após conectar ao servidor é enviada uma mensagem para o mesmo com o nick des cliente
    //** Se o servidor enviar '0' como resposta entao o nick está sendo usado e o cliente fecha

    strcpy(comando  , "2 ");
    strcat(comando  , nick);
    strcat(comando  , " ");
    strcat(comando  , argv[1]);
	
    /*Enviando a mensagem*/

    conexao = sendto(soquet, comando, msg_limit +1, 0, (struct sockaddr *) &servidor_addr, sizeof(servidor_addr));

    /*Espera a resposta*/

    recvfrom(soquet, msg, msg_limit+1, 0, NULL , NULL);

    /*Caso 0 fecha o programa*/	

    if(msg[0] == '0')
    {
        printf("nome de usuario ja esta sendo usado %s \n" , msg);
        return 0;
    }

    /*****************************************************************************************/
    /********************************** LANÇANDO A THREAD ************************************/

    int x;

    if(pthread_create(&thread, NULL, escutando , &x))
    {

        printf("Erro ao criar a thread\n");
        return 0;
    }

    /*****************************************************************************************/

    /************************************* LAGO PRINCIPAL ************************************/
    //** Irá capturar os comandos POST, MUTE, UNMUTE E CLOSE 
    //** POST SEQUE O FORMATO:    POST <TEXTO>
    //** MUTE SEQUE O FORMATO:    MUTE <NICK ALVO>             * detalhe:  Qualquer um pode se auto mutar   
    //** UNMUTE SEQUE O FORMATO:  UNMUTE <NICK ALVO>   	       * detalhe:  Qualquer um pode se auto desmutar, os desmute funciona mesmo se o alvo nao estiver mutado
    //** CLOSE SEQUE O FORMATO:   CLOSE				

    //**  FUNÇÕES
    //** POST <TEXTO> : ENVIA A MENSAGEM TEXTO PARA O SERVIDOR A QUAL IRÁ REPASSAR PARA TODOS OS DEMAIS USUARIOS
    //** MUTE <NICK ALVO>: IRÁ MUTAR O USUARIO, FAZENDO QUE ESTE CLIENTE NAO RECEBE MAIS MENSAGENS DOS POST DO USUARIO ALVO
    //** UNMUTE <NICK ALVO>: IRÁ DESMUTAR O USUARIO, FAZENDO QUE ESTE CLIENTE RECEBA MENSAGENS DOS POST DO USUARIO ALVO
    //** CLOSE: IRÁ ENCERRAR O CONEXAO E ESTE CLIENTE, NO SERVIDOR O NOME DESTE USUARIO SERÁ APAGADO  
    //** detalhe: se este cliente encerrar sem o close o usuario deste cliente ainda permanecerá no servidor

    while(1)
    {

        printf("\n");
        scanf("%[^\n]s", comando);
        setbuf(stdin, NULL);

	
        if(comando[0] == 'P' && comando[1] == 'O' && comando[2] == 'S' && comando[3] == 'T')  // POST <TEXTO>
        {
            strcpy(s, comando);
            char* token = strtok(s, " ");

	    // PARA O SERVIDOR A MENSAGEM É ENVIADA COMO: 1 <NICK CLIENTE> <TEXTO> , ONDE 1 SIGNIFICA POST
            strcpy(msg , "1 ");
            strcat(msg , nick);
            strcat(msg , " ");
            token = strtok(NULL, " ");

            while (token)
            {
                strcat(msg , token);
                strcat(msg , " ");
                token = strtok(NULL, " ");
            }

            conexao = sendto(soquet, msg, msg_limit +1, 0, (struct sockaddr *) &servidor_addr, sizeof(servidor_addr));

        }
        else if(comando[0] == 'M' && comando[1] == 'U' && comando[2] == 'T' && comando[3] == 'E')  // MUTE <NICK>
        {

            strcpy(s, comando);
            char* token = strtok(s, " ");

	    // PARA O SERVIDOR A MENSAGEM É ENVIADA COMO: 3 <NICK CLIENTE> <NICK ALVO> , ONDE 3 SIGNIFICA MUTE
            strcpy(msg , "3 ");
            strcat(msg , nick);
            strcat(msg , " ");

            token = strtok(NULL, " ");

            while (token)
            {
                strcat(msg , token);
                strcat(msg , " ");
                token = strtok(NULL, " ");
            }

            conexao = sendto(soquet, msg, msg_limit +1, 0, (struct sockaddr *) &servidor_addr, sizeof(servidor_addr));

        }
        else if(comando[0] == 'U' && comando[1] == 'N' && comando[2] == 'M' && comando[3] == 'U' && comando[4] == 'T' && comando[5] == 'E')  // UNMUTE <NICK>
        {

            strcpy(s, comando);
            char* token = strtok(s, " ");

	    // PARA O SERVIDOR A MENSAGEM É ENVIADA COMO: 5 <NICK CLIENTE> <NICK ALVO> , ONDE 5 SIGNIFICA UNMUTE
            strcpy(msg , "5 ");
            strcat(msg , nick);
            strcat(msg , " ");

            token = strtok(NULL, " ");

            while (token)
            {
                strcat(msg , token);
                strcat(msg , " ");
                token = strtok(NULL, " ");
            }

            conexao = sendto(soquet, msg, msg_limit +1, 0, (struct sockaddr *) &servidor_addr, sizeof(servidor_addr));

        }
        else if(comando[0] == 'C' && comando[1] == 'L' && comando[2] == 'O' && comando[3] == 'S' && comando[4] == 'E')  // CLOSE
        {
            char msg[600];

	    // PARA O SERVIDOR A MENSAGEM É ENVIADA COMO: 4 <NICK>, ONDE 4 SIGNIFICA CLOSE
            strcpy(msg, "4");
            strcat(msg, " ");
            strcat(msg, nick);

            conexao = sendto(soquet, msg, msg_limit +1, 0, (struct sockaddr *) &servidor_addr, sizeof(servidor_addr));
            close(soquet);
	    return 0;
            break;
        }
        else // COMANDO INVALIDO
        {
            printf("Comando Invalido\n");
        }

    }

    //***********************************ESPERANDO A THREAD ACABAR ******************************/

    if(pthread_join(thread, NULL))
    {
        printf("Erro na thread\n");
        return 1;
    }

    /********************************************************************************************/
    return 1;
}

