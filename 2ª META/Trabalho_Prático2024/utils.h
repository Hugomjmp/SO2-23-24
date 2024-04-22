#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <string.h>


#define NOME TEXT("\\\\.\\pipe\\BOLSA")
#define UTILIZADORES_REGISTADOS "Clientes.txt"

//DADOS DA ESTRUTURA NAMEDPIPE BOLSA
#define TAM 20

typedef struct
{
	TCHAR login[TAM];
	TCHAR password[TAM];
	TCHAR comando[300];
	TCHAR RESPOSTA[300];

}clienteData;

DWORD WINAPI trataComandosClientes();
DWORD WINAPI verificaClientes();

void CriaRegedit();
DWORD leRegedit();
void escreveRegedit();