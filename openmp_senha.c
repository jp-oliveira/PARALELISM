#include <stdio.h>
#ifdef _OPENMP
	#include <omp.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

char char_aleatorio(int);
char gerasenha(char [], int);
char verificasenha(char []);
char verifica_acerto(char [], char [], int);

int main(void){

    printf("\nQuantas senhas voce quer adivinhar?\n");
    int quantidadesenhas;
    scanf("%d",&quantidadesenhas);

    printf("\nQual tamanho cada senha tera?\n");
    int limite;
    scanf("%d",&limite);
	
	struct timeval comeco_paralelo, fim_paralelo, comeco_iteracao, fim_iteracao;
    printf("\nComecando a adivinhacao cautelosa.\n");
    srand(time(NULL));
    
	int iterar = 0;    //iterar o for principal
	char senha[limite];
	char copia[limite];
	double sumparalela = 0.0;
	double tempos[4];
	gettimeofday(&comeco_paralelo, NULL);
	
	#pragma omp parallel private(iterar, senha, copia, comeco_iteracao, fim_iteracao) shared(quantidadesenhas, limite, tempos) reduction(+: sumparalela)
	{
		#pragma omp for
			for(iterar = 0; iterar < quantidadesenhas; iterar++){
				gettimeofday(&comeco_iteracao, NULL);
				gerasenha(senha, limite);
				verifica_acerto(senha, copia, limite);
				gettimeofday(&fim_iteracao, NULL);
				sumparalela += (double)(fim_iteracao.tv_usec - comeco_iteracao.tv_usec)/1000000 + (double)(fim_iteracao.tv_sec - comeco_iteracao.tv_sec);
				
				if(omp_get_thread_num() == 0)
					tempos[0] += (double)(fim_iteracao.tv_usec - comeco_iteracao.tv_usec)/1000000 + (double)(fim_iteracao.tv_sec - comeco_iteracao.tv_sec);
				
				else if(omp_get_thread_num() == 1)
					tempos[1] += (double)(fim_iteracao.tv_usec - comeco_iteracao.tv_usec)/1000000 + (double)(fim_iteracao.tv_sec - comeco_iteracao.tv_sec);
				
				else if(omp_get_thread_num() == 2)
					tempos[2] += (double)(fim_iteracao.tv_usec - comeco_iteracao.tv_usec)/1000000 + (double)(fim_iteracao.tv_sec - comeco_iteracao.tv_sec);
					
				else if(omp_get_thread_num() == 3)
					tempos[3] += (double)(fim_iteracao.tv_usec - comeco_iteracao.tv_usec)/1000000 + (double)(fim_iteracao.tv_sec - comeco_iteracao.tv_sec);
			}
	}
	gettimeofday(&fim_paralelo, NULL);    
	printf("\nO Tempo total gasto dentro da regiao paralela: %fs\n",sumparalela);
	printf("\nO Tempo total gasto dentro da regiao paralela na thread zero: %fs\n",tempos[0]);
	printf("\nO Tempo total gasto dentro da regiao paralela na thread um: %fs\n",tempos[1]);
	printf("\nO Tempo total gasto dentro da regiao paralela na thread dois: %fs\n",tempos[2]);
	printf("\nO Tempo total gasto dentro da regiao paralela na thread tres: %fs\n",tempos[3]);
    
    printf("\nO Tempo gasto na execucao do programa paralelo: %fs\n\n",(double)(fim_paralelo.tv_usec - comeco_paralelo.tv_usec)/1000000 + (double)(fim_paralelo.tv_sec - comeco_paralelo.tv_sec));
    return 0;
}

char gerasenha(char senha[], int limite){
	int validou = 0;        //flag para se certificar que a senha tem todos os tipos de caracter.

	while(validou != 1){   // VERIFICA SE A SENHA TEM CARACTER MAIUSCULO, MINUSCULO, NUMERO E SIMBOLO
		
		for(int i = 0; i < limite; i++){
			senha[i] = char_aleatorio(rand() % 4);
		}
		//printf("\nlimite = %d, tamanho da senha eh %d\n",limite,strlen(senha));
		
		if (verificasenha(senha) == 's'){
			printf("\nA senha gerada foi: %s. Essa senha eh valida! Estamos na thread numero %d\n",senha,omp_get_thread_num());
			validou = 1;
		}
		else{
			printf("\nA senha gerada foi: %s. Essa senha nao eh valida! Estamos gerando outra. Estamos na thread numero %d\n",senha,omp_get_thread_num());
		}
	}
}

char verifica_acerto( char senha[], char copia[], int limite){  
	for( int cont = 0; cont != limite; cont++){   
		char chute;
		while(chute != senha[cont]){
			chute = char_aleatorio(rand() % 4);
			//printf("%c",chute); // para ver ele acertando
		}
		copia[cont] = chute;
	}
	printf("\nAcertou a senha = %s! Estamos na thread numero %d!\n", copia,omp_get_thread_num());
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