#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <string.h>



#define BUFFTAM 512

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

HKEY trataRegedit();