#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
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

DWORD WINAPI trataComandosClientes();
DWORD WINAPI verificaClientes();

void CriaRegedit();
DWORD leRegedit();
void escreveRegedit();