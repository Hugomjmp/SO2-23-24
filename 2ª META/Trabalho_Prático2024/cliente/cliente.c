#include "..\\utils.h"


int _tmain(int argc, TCHAR* argv[])
{
	//clienteData CD;
	TCHAR login[TAM], password[TAM], comando[200];

	//variaveis
	BOOL resultado;
	DWORD nBytes;

	//HANDLES
	HANDLE hPipe;

	//Nome do namedpipe
	LPCTSTR pBolsa = TEXT("\\\\.\\pipe\\BOLSA");


	//Codigo para o unicode
#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	//_setmode(_fileno(stderr), _O_WTEXT);
#endif
	_tprintf(TEXT("#################################################################\n"));
	_tprintf(TEXT("#\t\t\t\t\t\t\t\t#\n"));
	_tprintf(TEXT("#\t\tBOLSA DE VALORES ONLINE\t\t\t\t#\n"));
	_tprintf(TEXT("#\t\t\t\t\t\t\t\t#\n"));
	_tprintf(TEXT("#################################################################\n"));

	//Inicializar o NAMEDPIPE
	/*hPipe = CreateFile(
		pBolsa,							//lpFileName,
		GENERIC_WRITE | GENERIC_READ,	//dwDesiredAccess,
		0,								//dwShareMode,
		NULL,							//lpSecurityAttributes,
		OPEN_EXISTING,					//dwCreationDisposition,
		FILE_ATTRIBUTE_NORMAL,          //dwFlagsAndAttributes,
		NULL							//hTemplateFile
	);
	if (hPipe == INVALID_HANDLE_VALUE)
	{
		_tprintf(TEXT("\n[ERRO] O servidor Bolsa não está em funcionamento, %d."), GetLastError());
		return -1;
	}
	resultado = WaitNamedPipe(hPipe, INFINITE);*/
	//CREDENCIAIS DO UTILIZADOR
	_tprintf(TEXT("\nLogin: "));
	_tscanf(TEXT("%s"), &login);
	_tprintf(TEXT("\nPassword: "));
	_tscanf(TEXT("%s"), &password);
	// DEBUG...
	_tprintf(TEXT("\nLi -> Login: %s com tamanho %d e Password: %s com tamanho %zu"), login, _tcslen(login), password, _tcslen(password));

	//COMANDOS DO CLIENTE

	while ((_tcsicmp(TEXT("exit"), comando)) != 0)
	{
		_tprintf(TEXT("\nComando: "));
		_tscanf(TEXT("%s"), &comando);
		_tprintf(TEXT("\nLi -> Comando: %s com tamanho %zu"), comando, _tcslen(comando));
		/*resultado = WriteFile(
			hPipe,
			comando,
			(_tcslen(comando) + 1) * sizeof(TCHAR),
			&nBytes,
			NULL
		);*/
		//comando[_tcslen(comando) - 1] = '\0'; //retirar o /0

	}

	//CloseHandle(hPipe);
	return 0;
}

