#include <stdio.h>
#include <mpi.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

char char_aleatorio(int);
char gerasenha(char [], int, int);
char verificasenha(char []);
char verifica_acerto(char [], char [], int, int);

int main(int argc, char *argv[]){

	struct timeval comeco_paralelo, fim_paralelo, comeco_iteracao, fim_iteracao;
    printf("\nComecando a adivinhacao cautelosa.\n");
    srand(time(NULL));
    
	/*
	int MPI_Send( void *buf, int count, MPI_Datatype datatype, int dest,   int tag, MPI_Comm comm)
	int MPI_Recv( void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status)
	*/
	gettimeofday(&comeco_paralelo, NULL);
	int inicio, fim,limite,i, n_bar,p, n, my_rank; //i = para iterar no for.
	int tag = 0;   //valor fixo.
	char senha[atoi(argv[2])];
	char copia[atoi(argv[2])];
	double sumparalela = 0.0;
	
	MPI_Status status;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &p);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	
	if (my_rank == 0) {
        n = atoi(argv[1]);
		limite = atoi(argv[2]);

		for (i=1; i<p; i++){
			MPI_Send(&n, 1, MPI_INT, i, tag, MPI_COMM_WORLD);       //envia para todos os processos o valor de n e limite
			MPI_Send(&limite, 1, MPI_INT, i, tag, MPI_COMM_WORLD);
		}
    }
	else{
		MPI_Recv(&n, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status); //todos os processos recebem o valor de n do processo 0
		MPI_Recv(&limite, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
	}
	
	n_bar = n/p;    //divide a precisão de n pelo numero de processos p/ cada processo ter +/- o mesmo trabalho
    inicio = my_rank * n_bar;
    fim = (my_rank + 1) * n_bar;
	double localsum = 0.0;
	
	for(i = inicio; i < fim; i++){
		gettimeofday(&comeco_iteracao, NULL);
		gerasenha(senha, limite, my_rank);
		verifica_acerto(senha, copia, limite, my_rank);
		gettimeofday(&fim_iteracao, NULL);
		sumparalela += (double)(fim_iteracao.tv_usec - comeco_iteracao.tv_usec)/1000000 + (double)(fim_iteracao.tv_sec - comeco_iteracao.tv_sec);
		//printf("\nSenha paralela eh igual a %s e essa thread eh a %d\n", senha, i);
		//printf("\nSum paralela eh igual a %f\n", sumparalela);
	}
	//printf("\nProcesso %d sumparalel é igual a %f\n", my_rank, sumparalela);  //mostra o tempo que cada processo levou em cada for
	if (my_rank != 0) {
		MPI_Send(&sumparalela, 1, MPI_DOUBLE, 0, tag, MPI_COMM_WORLD); //todos os outros processo mandam sum p/ 0
		//printf("\nProcesso 0 enviou %f\n", sumparalela);
	}
	else{
		for(i = 1; i < p; i++){
			MPI_Recv(&localsum, 1, MPI_DOUBLE, i, tag, MPI_COMM_WORLD, &status); //recebe o valor de localsum do processo 0
			//printf("\nProcesso %d recebeu %f\n", i, localsum);
			sumparalela += localsum;
		}
	}
	//sum paralela é igual a soma dos tempos gastos SOMENTE dentro do for
	/*o tempo de baixo é igual ao tempo gasto na aplicação inteira INCLUINDO troca de mensagens - esse tempo é paralelizado, logo
	esse tempo deve ser menor que o sum paralela.*/
	MPI_Finalize();
	gettimeofday(&fim_paralelo, NULL);
	if(my_rank == 0){
		printf("\nO TEMPO TOTAL GASTO DENTRO DA REGIAO PARALELA: %fs\n",sumparalela); 
		printf("\nO TEMPO GASTO NO PROCESSO 0 - MASTER: %fs\n\n",(double)(fim_paralelo.tv_usec - comeco_paralelo.tv_usec)/1000000 + (double)(fim_paralelo.tv_sec - comeco_paralelo.tv_sec));
    }
	return 0;
}

char gerasenha(char senha[], int limite, int my_rank){
	int validou = 0;        //flag para se certificar que a senha tem todos os tipos de caracter.

	while(validou != 1){   // VERIFICA SE A SENHA TEM CARACTER MAIUSCULO, MINUSCULO, NUMERO E SIMBOLO
		
		for(int i = 0; i < limite; i++){
			senha[i] = char_aleatorio(rand() % 4);
		}
		//printf("\nlimite = %d, tamanho da senha eh %d\n",limite,strlen(senha));
		
		if (verificasenha(senha) == 's'){
			printf("\nA senha gerada foi: %s. Essa senha eh valida! Estamos no processo numero %d\n",senha,my_rank);
			validou = 1;
		}
		else{
			printf("\nA senha gerada foi: %s. Essa senha nao eh valida! Estamos gerando outra. Estamos no processo numero %d\n",senha,my_rank);
		}
	}
}
char verifica_acerto( char senha[], char copia[], int limite, int my_rank){  
	for( int cont = 0; cont != limite; cont++){   
		char chute;
		while(chute != senha[cont]){
			chute = char_aleatorio(rand() % 4);
			//printf("%c",chute); // para ver ele acertando
		}
		copia[cont] = chute;
	}
	printf("\nAcertou a senha = %s! Estamos no processo numero %d!\n", copia,my_rank);
}
char verificasenha(char senha[]){
    int maiuscula = 0;   //para saber se tem um maiusculo, um minusculo e um caracter especial;
    int numero = 0;
    int simbolo = 0;
    int minuscula = 0;
    char copia[strlen(senha)];
    strcpy(copia,senha);

    for(int i = 0; i < strlen(senha); i++){
        if('A' <= copia[i] && copia[i] <= 'Z'){
            maiuscula = 1;
        }
        else if('0' <= copia[i] && copia[i] <= '9'){
            numero = 1;
        }
        else if('a' <= copia[i] && copia[i] <= 'z'){
            minuscula = 1;
        }
        else{
            simbolo = 1;   
        }
    }
    
    if(maiuscula + minuscula + numero + simbolo == 4) 
        return 's';
    else
        return 'n';
}
char char_aleatorio(int argumento){
	char maiusculo[] = "ABCDEFGHIJKLMNOPQRSTUWVXYZ";
    char minusculo[] = "abcdefghijklmnopqrstuwvxyz";
    char numero[]    = "0123456789";
    char simbolo[]   = "!@#$&*";
    char letra;
    int sorteio;

    if (argumento == 0){  // para maiusculo
        sorteio = rand() % 27;
	    letra = maiusculo[sorteio];
	}
    else if (argumento == 1){  // para maiusculo
        sorteio = rand() % 27;
	    letra = minusculo[sorteio];
	}
    else if (argumento == 2){  // para maiusculo
        sorteio = rand() % 10;
	    letra = numero[sorteio];
	}
    else if (argumento == 3){  // para maiusculo
        sorteio = rand() % 6;
	    letra = simbolo[sorteio];
	}
    return letra;	
}