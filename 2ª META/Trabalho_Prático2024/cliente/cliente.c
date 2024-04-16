#include "..\\utils.h"


int _tmain(int argc, TCHAR* argv[])
{
	//clienteData CD;
	TCHAR login[20], password[20], comando[200];

	//HANDLES
	HANDLE hPipe;

	//Nome do namedpipe
	LPCTSTR pBolsa = TEXT("\\\\.\\pipe\\BOLSA");

	//Codigo para o unicode
	#ifdef UNICODE
		_setmode(_fileno(stdin), _O_WTEXT);
		_setmode(_fileno(stdout), _O_WTEXT);
	#endif
		_tprintf(TEXT("#################################################################\n"));
		_tprintf(TEXT("#\t\t\t\t\t\t\t\t#\n"));
		_tprintf(TEXT("#\t\tBOLSA DE VALORES ONLINE\t\t\t\t#\n"));
		_tprintf(TEXT("#\t\t\t\t\t\t\t\t#\n"));
		_tprintf(TEXT("#################################################################\n"));
		//CREDENCIAIS DO UTILIZADOR
		_tprintf(TEXT("\nLogin: "));
		_tscanf(TEXT("%s"), &login);
		_tprintf(TEXT("\nPassword: "));
		_tscanf(TEXT("%s"), &password);
		// DEBUG...
		_tprintf(TEXT("\nLi -> Login: %s com tamanho %d e Password: %s com tamanho %zu"), login, _tcslen(login), password, _tcslen(password));
		
		
		
		//Inicializar o NAMEDPIPE
		hPipe = CreateNamedPipe(
			pBolsa,					  // Nome do pipe
			PIPE_ACCESS_OUTBOUND,     // acesso em modo de escrita
			PIPE_TYPE_MESSAGE |       // message type pipe 
			PIPE_READMODE_MESSAGE |   // message-read mode 
			PIPE_WAIT,                // blocking mode 
			PIPE_UNLIMITED_INSTANCES, // max. instances  
			BUFFTAM,                  // output buffer size 
			BUFFTAM,                  // input buffer size 
			0,                        // client time-out 
			NULL);                    // default security attribute 
			
		WaitNamedPipe(hPipe, INFINITE);
		if (hPipe == INVALID_HANDLE_VALUE)
		{
			_tprintf(TEXT("\n[ERRO] Acesso ao bolsa pipe falhou, %d."), GetLastError());
			return -1;
		}
		//COMANDOS DO CLIENTE
		
		while ((_tcsicmp(TEXT("exit"), comando)) != 0)
		{
			_tprintf(TEXT("\nComando: "));
			_tscanf(TEXT("%s"), &comando);
			_tprintf(TEXT("\nLi -> Comando: %s com tamanho %zu"), comando, _tcslen(comando));
			//comando[_tcslen(comando) - 1] = '\0'; //retirar o /0
			
		}
		
		CloseHandle(hPipe);
		return 0;
}

