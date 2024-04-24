#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <io.h>
#include <string.h>


#define NOME TEXT("\\\\.\\pipe\\BOLSA")
#define UTILIZADORES_REGISTADOS "Clientes.txt"
#define EMPRESAS "Empresas.txt"

//DADOS DA ESTRUTURA NAMEDPIPE BOLSA
#define TAM 20
#define MAX_LINHA 30
#define MAX_COLUNA 3 

 //Shared Memory
#define SHM_NAME TEXT("SHM_BOLSA")
#define SEM_WRITE_NAME TEXT("SEM_WRITE")
#define SEM_READ_NAME TEXT("SEM_READ")

//SINCRONIZAÇÃO
#define EVENT_NAME TEXT("EVENT_BOLSA")

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
	TCHAR user[50];
	float saldo;
	BOOL estado;
}userData;

typedef struct {
	empresaData* sharedMem;
	HANDLE hMapFile;
}ControlData;

//THREADS
DWORD WINAPI trataComandosClientes();
DWORD WINAPI verificaClientes();
DWORD WINAPI variaPreços(LPVOID empresas);

//Prototipos das funções
void CriaRegedit();
DWORD leRegedit();
void escreveRegedit();