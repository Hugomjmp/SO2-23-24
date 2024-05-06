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

//SINCRONIZA��O
#define EVENT_NAME TEXT("EVENT_BOLSA")
#define EVENT_NAME_O TEXT("EVENT_BOLSA_O")
#define EVENT_NAME_C TEXT("EVENT_CLIENTE")
#define EVENT_NAME_V TEXT("EVENT_VERIFICA")

//SEMAFOROS
#define SEM_BOLSA TEXT("SEM_BOLSA")
#define SEM_CLIENT_NAME TEXT("SEM_VAGA")
#define	SEM_CLIENTE_LOGIN TEXT("SEM_LOGIN")
#define SEM_BUYSELL TEXT("SEM_BUYSELL")

//MUTEX
#define MUTEX_NAME TEXT("MUTEX_BOLSA")
#define MUTEX_NAME_O TEXT("MUTEX_BOARD")
#define MUTEX_NAME_C TEXT("MUTEX_CLIENTE")
//NAMEDPIPE
#define NAME_PIPE TEXT("\\\\.\\pipe\\BOLSA")


//ESTRUTURAS
typedef struct {
	TCHAR nomeEmpresa[50];
	DWORD nA��es;
	float pA��o;
	
}empresaData;

typedef struct   
{
	TCHAR login[TAM];
	TCHAR password[TAM];
	TCHAR comando[300];
}clienteData;

typedef struct {
	TCHAR RESPOSTA[300];
	empresaData empW[30];
}clienteResposta;

typedef struct {
	TCHAR username[TAM];
	TCHAR password[TAM];
	float saldo;
	BOOL estado;
}userData;

typedef struct {
	TCHAR nomeEmpresa[50];
	TCHAR username[TAM];
	DWORD nA��es;
	float valor;
}carteiraAcoes;
typedef	struct {
	TCHAR EmpresaNome[50];
	DWORD nAcoes;
	float pAcao;
}UltimaTransacao;
typedef struct {
	BOOL pause;
}ControlPause;
typedef struct {
	empresaData empresas[30];
	carteiraAcoes cartAcoes[30];
	UltimaTransacao ultmTransacao[1];
}boardData;

typedef struct {
	empresaData* empresas;
	userData* users;
	carteiraAcoes* cartAcoes;
	ControlPause* ctrPause;
}ControlData;

typedef struct {
	HANDLE hPipe[10];
	HANDLE hTrinco;
}tDataInfo;
typedef struct {
	tDataInfo* ptd;
	DWORD id;
	empresaData* empresas;
	userData* users;
	carteiraAcoes* cartAcoes;
	ControlPause* ctrPause;
}tDataInfo_EXTRA;

typedef struct {
	HANDLE hPipe;
	BOOL continua;
	HANDLE hEv;
	clienteData* clidData;
	clienteResposta* cliRes;
}DATA;

//THREADS
DWORD WINAPI trataComandosClientes(LPVOID data);
DWORD WINAPI verificaClientes(LPVOID ctrlData);
DWORD WINAPI variaPre�os(LPVOID empresas);
DWORD WINAPI Organiza_dados(LPVOID empresas);
DWORD WINAPI SMtoLocal(LPVOID empresas);
DWORD WINAPI recebeMSG(LPVOID data);

//Prototipos das fun��es
void CriaRegedit();
DWORD leRegedit();
void escreveRegedit();
void mostraMenu();
void apresentacao();
void mostra_tabela(empresasBoard, boardDt);
void mostraTitulo();
void mostraMenuCliente();
void avisos(int x);
