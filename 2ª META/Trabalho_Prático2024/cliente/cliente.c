#include "..\\utils.h"


int _tmain(int argc, TCHAR* argv[])
{
	//clienteData CD;
	TCHAR login[20], password[20], comando[200];

	//HANDLES
	HANDLE hpipe;

	//Nome do namedpipe
	LPCTSTR pBolsa = TEXT("..\\bolsa");

	//Codigo para o unicode
	#ifdef UNICODE
		_setmode(_fileno(stdin), _O_WTEXT);
		_setmode(_fileno(stdout), _O_WTEXT);
	#endif

		//CREDENCIAIS DO UTILIZADOR
		_tprintf(TEXT("Login: "));
		_fgetts(login, 20, stdin);
		_tprintf(TEXT("\nPassword: "));
		_fgetts(password, 20, stdin);
		// DEBUG...
		_tprintf(TEXT("\nLi -> Login: %s com tamanho %d e Password: %s com tamanho %d"), login, _tcslen(login)-1, password, _tcslen(password)-1);
		
		//Inicializar o NAMEDPIPE
		hpipe = CreateNamedPipe(
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

		if (hpipe == INVALID_HANDLE_VALUE)
		{
			_tprintf(TEXT("\n[ERRO] Acesso ao bolsa pipe falhou, %d."), GetLastError());
			return -1;
		}
		//COMANDOS DO CLIENTE
		
		while ((_tcsicmp(TEXT("exit"), comando)) != 0)
		{
			_tprintf(TEXT("\nComando: "));
			_fgetts(comando, 200, stdin);
			_tprintf(TEXT("\nLi -> Comando: %s com tamanho %d"), comando, _tcslen(comando) - 1);
			comando[_tcslen(comando) - 1] = '\0'; //retirar o /0
			
		}
		
		CloseHandle(hpipe);
		return 0;
}

