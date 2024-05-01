#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <io.h>
#include <string.h>



#define UTILIZADORES_REGISTADOS "Clientes.txt"
#define EMPRESAS "Empresas.txt"

//DADOS DA ESTRUTURA NAMEDPIPE BOLSA
#define TAM 30
#define MAX_EMPRESAS 30
#define MAX_CLIENTES 5
//Processo Board

 //Shared Memory
#define SHM_NAME TEXT("SHM_BOLSA")
#define SEM_WRITE_NAME TEXT("SEM_WRITE")
#define SEM_READ_NAME TEXT("SEM_READ")

//SINCRONIZAÇÃO
#define EVENT_NAME TEXT("EVENT_BOLSA")
#define EVENT_NAME_O TEXT("EVENT_BOLSA_O")
#define EVENT_NAME_C TEXT("EVENT_CLIENTE")

//SEMAFOROS
#define SEM_BOLSA TEXT("SEM_BOLSA")
#define SEM_CLIENT_NAME TEXT("SEM_VAGA")


//MUTEX
#define MUTEX_NAME TEXT("MUTEX_BOLSA")
#define MUTEX_NAME_O TEXT("MUTEX_BOARD")
#define MUTEX_NAME_C TEXT("MUTEX_CLIENTE")
//NAMEDPIPE
#define NAME_PIPE TEXT("\\\\.\\pipe\\BOLSA")


//ESTRUTURAS
typedef struct   
{
	TCHAR login[TAM];
	TCHAR password[TAM];
	TCHAR comando[300];
	TCHAR RESPOSTA[300];
}clienteData;

typedef struct {
	TCHAR nomeEmpresa[50];
	DWORD nAções;  
	float pAção;
}empresaData;

typedef struct {
	TCHAR username[TAM];
	TCHAR password[TAM];
	float saldo;
	BOOL estado;
}userData;

typedef struct {
	empresaData* empresas;
	userData* users;
	OVERLAPPED ov;
	HANDLE hPipe[5];
	DWORD id;
	//HANDLE hEvent[5];
}ControlData;

//THREADS
DWORD WINAPI trataComandosClientes(LPVOID ctrlData);
DWORD WINAPI verificaClientes(LPVOID ctrlData);
DWORD WINAPI variaPreços(LPVOID empresas);
DWORD WINAPI Organiza_dados(LPVOID empresas);
DWORD WINAPI SMtoLocal(LPVOID empresas);
DWORD WINAPI cliente_read();

//Prototipos das funções
void CriaRegedit();
DWORD leRegedit();
void escreveRegedit();
void mostra_tabela(empresaData* empresasBoard);
void apresentacao();

