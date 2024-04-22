#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <string.h>


#define BUFFTAM 512
#define UTILIZADORES_REGISTADOS "..\\Trabalho_Prático2024\Clientes.txt"
//DADOS DA ESTRUTURA NAMEDPIPE BOLSA
#define TAM 20

typedef struct
{
	TCHAR login[TAM];
	TCHAR password[TAM];
	TCHAR comando[300];
	TCHAR RESPOSTA[300];

}clienteData;

DWORD WINAPI trataComandosClientes(LPVOID hPipe);
DWORD WINAPI verificaClientes(LPVOID hPipe);

HKEY trataRegedit();