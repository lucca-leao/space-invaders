#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <thread>
#include <string.h>
#include <cstdlib>
#include <time.h>
#include <ctime>
#include <vector>
#include <fstream>
#include <mutex>
#include <boost/interprocess/anonymous_shared_memory.hpp>
#include <boost/interprocess/mapped_region.hpp>

using namespace std;
using namespace boost::interprocess;

const int tam_mapa = 20;
int contador = 0;
int contador_tiro_invasor = 0;
int pos_jogador = tam_mapa/2;
int tiro_inmigo_x = 0;
int tiro_inmigo_y = 0;
bool tiro_disparado = false;
bool tiro_inmigo = false;
int pos_tiro_x = 0;
int pos_tiro_y = 0;
bool game_over = false;
bool vitoria = false;
bool derrota = false;
char buffer[80];
int pontuacao = 0;
mutex mapa_m;
mutex pos_tiro_x_m;
mutex pos_tiro_y_m;
mutex pos_jogador_m;
mutex tiro_inmigo_y_m;
mutex tiro_inmigo_x_m;
static int *pontuacao_ipc;
bool pos_invasores[tam_mapa][tam_mapa];
char mapa[tam_mapa][tam_mapa] ={
	"|-----------------|",
	"|                 |",
	"|                 |",
	"|                 |",
	"|                 |",
	"|                 |",
	"|                 |",
	"|                 |",
	"|                 |",
	"|                 |",
	"|                 |",
	"|                 |",
	"|                 |",
	"|                 |",
	"|                 |",
	"|-   -   -   -   -|",
	"|                 |",
	"|                 |",
	"|-----------------|",
};

char mapa_original[tam_mapa][tam_mapa] ={
	"|-----------------|",
	"|                 |",
	"|                 |",
	"|                 |",
	"|                 |",
	"|                 |",
	"|                 |",
	"|                 |",
	"|                 |",
	"|                 |",
	"|                 |",
	"|                 |",
	"|                 |",
	"|                 |",
	"|                 |",
	"|-   -   -   -   -|",
	"|                 |",
	"|                 |",
	"|-----------------|",
};

//thread jogador
void jogador(){
	char comando;
	while(!game_over){
	system("stty -echo");
	system("stty cbreak");
	comando = getchar();
	if(comando == 's' || comando == 'S'){
		if(pos_jogador <= 1){
			pos_jogador = pos_jogador;
		}
		else{
			pos_jogador = pos_jogador - 1; // tecla S move a nave para a esquerda
		}
	}
	else if(comando == 'd' || comando == 'D'){
		if(pos_jogador >=tam_mapa-3){
			pos_jogador = pos_jogador;
		}
		else{
			pos_jogador = pos_jogador + 1; // tecla D move a nave para a direita
		}
	}
	else if(comando == ' '){ //barra de espaço dispara um tiro
		if(!tiro_disparado){
			tiro_disparado = true;
			pos_tiro_x = pos_jogador;
			pos_tiro_y = 17;
		}
	}
	system("stty echo");
	system("stty -cbreak");
	}
}

//thread invasores
void invasores(){
	bool aux_pos_invasores[tam_mapa][tam_mapa];
	int invasores_vivos = 0;
	int shifts = 0;
	while(!game_over){
		if(shifts == 11){
			game_over = true;
			derrota = true;
		}
		//condição para inimigos atirarem é baseado no número de vezes que a tela foi atualizada
		if((contador_tiro_invasor == 30) && (tiro_inmigo == false)){
			tiro_inmigo = true;
			tiro_inmigo_y = 3 + shifts;
			//qualquer invasor na fileira mais à frente pode atirar, escolhido aleatoriamente atrávés de rand()
			tiro_inmigo_x = rand() % 15 + 3;
			if(tiro_inmigo_x%2 != 0){
				tiro_inmigo_x++;
			}
			contador_tiro_invasor = 0;
		}
		if(contador >= 75){
			for(int i = 1; i < tam_mapa; i++){
				for(int j = 0; j < tam_mapa; j++){
					aux_pos_invasores[i][j] = pos_invasores[i-1][j];
				}
			}
			for(int k = 0; k < tam_mapa; k++){
				aux_pos_invasores[0][k] = false;
			}
		
			for(int a = 0; a < tam_mapa; a++){
				for(int b = 0; b < tam_mapa; b++){
					pos_invasores[a][b] = aux_pos_invasores[a][b];
				}
			}
		contador = 0;
		shifts++;
		}
		for(int m = 0; m < tam_mapa; m++){
			for(int n = 0; n < tam_mapa; n++){
				if(pos_invasores[m][n] == true){
					invasores_vivos++;
				}
			}
		}
		if(invasores_vivos == 0){
			game_over = true;
			vitoria = true;
		}
		invasores_vivos = 0;
	}
}


//thread que desenha a interface na tela
void draw(){
	//declara estruturas necessárias para suspender a thread por um periodo de 50ms
	int milisec = 50;
	struct timespec req = {0};
	req.tv_sec = 0;
	req.tv_nsec = milisec * 1000000L;
	while(!game_over){
		system("clear");
		//usa como base o mapa_original para escrever todos os elementos
		for(int t = 0; t < tam_mapa; t++){
			for(int y = 0; y < tam_mapa; y++){
				mapa[t][y] = mapa_original[t][y];
			}
		}
		//desenha os inimigos invasores
		for(int i = 0; i < tam_mapa; i++){
			for(int j = 0; j < tam_mapa; j++){
				if(pos_invasores[i][j] == true){
					mapa[i][j] = 'O';
				}
			}
		}
		//desenha o tiro, se for disparado
		if(tiro_disparado == true){
			//detecta colisão com os inimigos
			if(mapa[pos_tiro_y][pos_tiro_x] == 'O'){
				mapa[pos_tiro_y][pos_tiro_x] = ' ';
				pos_invasores[pos_tiro_y][pos_tiro_x] = false;
				tiro_disparado = false;
				pontuacao = pontuacao + 5;
			}
			if(pos_tiro_y == 1){
				tiro_disparado = false;
			}
			mapa[pos_tiro_y][pos_tiro_x] = '^';
			pos_tiro_y--;
		}
		//desenha o jogador
		mapa[tam_mapa-3][pos_jogador] = 'W';
		//desenha os tiros inimigos
		if(tiro_inmigo == true){
			//detecta colisão com o jogador
			if(mapa[tiro_inmigo_y][tiro_inmigo_x] == 'W'){
				game_over = true;
				derrota = true;
			}
			//testa se chegou no limite do mapa
			if(tiro_inmigo_y >= 18){
				tiro_inmigo = false;
			}
			mapa[tiro_inmigo_y][tiro_inmigo_x] = 'v';
			tiro_inmigo_y++;
		}
		//imprime o mapa na tela
		for(int a = 0; a < tam_mapa; a++){
			for(int b = 0; b < tam_mapa; b++){
				cout << mapa[a][b];
			}
			cout << endl;
		}
		cout << "Instruções:\nS: move para a esquerda\nD: move para a direita\nEspaco: atira\nPontuacao:" << pontuacao;
		cout << endl;
		contador++;
		contador_tiro_invasor++;
		//suspende a thread por 50ms
		nanosleep(&req, (struct timespec *)NULL);
	}
}

void inicializa_invasores(){
	for(int i = 0; i < tam_mapa; i++){
		for(int j = 0; j < tam_mapa; j++){
			if((i >= 2) && (i <= 5)){
				if((j%2 == 0) && ((j >=2) && (j <=tam_mapa-3))){
					pos_invasores[i][j] = true;
				}
				else{
					pos_invasores[i][j] = false;
				}
			}
			else{
				pos_invasores[i][j] = false;
			}
		}
	}
}

int main() 
{

	inicializa_invasores();
	int milisec = 100;
	struct timespec req = {0};
	req.tv_sec = 0;
	req.tv_nsec = milisec * 1000000L;
	//for(int i = 0; i < tam_mapa; i++){
	//	for(int j = 0; j < tam_mapa; j++){
	//		cout << pos_invasores[i][j];
	//	}
	//	cout << endl;
	//}

	std::thread t1(jogador);
	std::thread t2(invasores);
	std::thread t3(draw);
	t1.detach();
	t2.detach();
	t3.detach();
	//pega o tempo de incio em forma de string
	time_t rawtime;
    struct tm * timeinfo;
    time (&rawtime);
  	timeinfo = localtime(&rawtime);
  	strftime(buffer,sizeof(buffer),"%d-%m-%Y %H:%M:%S",timeinfo);
  	std::string inicio(buffer);


	//cria memória compartilhada em uma região mapeada para fazer comunicação inter processo
    mapped_region region(anonymous_shared_memory(1000));
	      //Write all the memory to 1
   	memset(region.get_address(), 1, region.get_size());

	struct timespec r = {0};
	r.tv_sec = 5;
	r.tv_nsec = 0;
	std::vector<int> p;
	int k = 0;
    while(!game_over){	
    	p.push_back(pontuacao);
    	k++;
    	nanosleep(&r, (struct timespec *)NULL);
    }

    //pega o tempo de término em forma de string
  	time_t raw;
    struct tm * info;
    time (&raw);
  	info = localtime(&raw);
  	strftime(buffer,sizeof(buffer),"%d-%m-%Y %H:%M:%S",info);
  	std::string termino(buffer);

    pid_t pid = fork(); //fork() lança um novo processo
    if(pid == 0){
    	//processo filho (logger)
    	//o filho pega os dados que estão salvos na memória compartilhada (region)
    	char *mem = static_cast<char*>(region.get_address());
    	ofstream outputFile("logger.txt");
    	outputFile << "Data e hora de inicio do jogo: " << inicio;
    	outputFile << endl;
    	for(int l = 0; l < p.size(); l++){
    		outputFile << "Pontuação corrente: " << p[l];
    		outputFile << endl;
    	}
    	outputFile << "Hora de término do jogo: " << termino;
    	outputFile << endl;
    	outputFile << "Pontuação final: " << pontuacao;
    }

    nanosleep(&req, (struct timespec *)NULL);
    if(vitoria){
    	cout << "\nGAME OVER!!! VOCE VENCEU\n";
    	cout << endl;
    	cout << "Pontuacao final:" << pontuacao;
    	cout << endl;
    }
    else if(derrota){
    	cout << "\nGAME OVER!!! VOCE PERDEU\n";
    	cout << endl;
    	cout << "Pontuacao final:" << pontuacao;
    	cout << endl;
    }
	
	return 0;
}