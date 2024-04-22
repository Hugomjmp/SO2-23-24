#include "..\\Trabalho_Prático2024\utils.h"

int _tmain(int argc, TCHAR* argv[])
{
	//HANDLES...
	HKEY hChaveClientes;
	HANDLE hPipe, hThreadArray[5];

	//Variáveis
	DWORD nAções, pAção = 0, nSegundos = 0;
	TCHAR comandoAdmin[200], nomeEmpresa[50];
	

	
#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	//_setmode(_fileno(stderr), _O_WTEXT);
#endif


	hChaveClientes = trataRegedit(); //TRATA DA VARIAVEL NCLIENTES

	//CRIAR O NAMEDPIPE BOLSA PARA INTERAÇÃO DOS CLIENTES
	hPipe = CreateNamedPipe(
			TEXT("\\\\.\\pipe\\BOLSA"),			  // Nome do pipe
			PIPE_ACCESS_DUPLEX,		  // acesso em modo de escrita e de leitura
			PIPE_TYPE_MESSAGE |       // message type pipe 
			PIPE_READMODE_MESSAGE |   // message-read mode 
			PIPE_WAIT,                // blocking mode 
			5,						  // max. instances  
			BUFFTAM,                  // output buffer size 
			BUFFTAM,                  // input buffer size 
			0,                        // client time-out 
			NULL);                    // default security attribute 
	if (hPipe == INVALID_HANDLE_VALUE)
	{
		_tprintf(TEXT("[ERRO] Falhou ao criar CreateNamedPipe, %d.\n"), GetLastError());
		return -1;
	}
	//CIRAR A THREAD PARA TRATAR OS COMANDOS DOS CLIENTES
	hThreadArray[0] = NULL; // é bom inicializar a zero para depois podermos testar se a thread foi criada com sucesso
	hThreadArray[0] = CreateThread(
						NULL,					// default security attributes
						0,						// use default stack size
						trataComandosClientes,	// thread function name
						&hPipe,					// argument to thread function
						0,						// use default creation flags
						NULL);
	/*hThreadArray[1] = NULL; // é bom inicializar a zero para depois podermos testar se a thread foi criada com sucesso
	hThreadArray[1] = CreateThread(
		NULL,					// default security attributes
		0,						// use default stack size
		verificaClientes,	// thread function name
		&hPipe,					// argument to thread function
		0,						// use default creation flags
		NULL);*/
	//COMANDO DO ADMINISTRADOR
	BOOL resultado;
	clienteData cd;
	DWORD nBytes;
	while ((_tcsicmp(TEXT("close"), comandoAdmin)) != 0)
	{
		
		ConnectNamedPipe(hPipe, INFINITE);
		resultado = ReadFile(
			hPipe,
			&cd,
			sizeof(cd),
			&nBytes,
			NULL
		);
		_tprintf(TEXT("Recebi login: %zu, password: %zu"), cd.login, cd.password);
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		_tprintf(TEXT("\nComando: "));
		//fflush(stdin);
		//_tscanf(TEXT("%s"), comandoAdmin);
		_tprintf(TEXT("\nLi -> Comando: %s com tamanho %zu"), comandoAdmin, _tcslen(comandoAdmin));

		//TRATA DO COMANNDO ACRESCENTAR UM EMPRESA
		if (_tcsicmp(TEXT("addc"),comandoAdmin,_tcslen(TEXT("addc"))) == 0)
		{
			_tscanf(TEXT("%s"), &nomeEmpresa);
			_tscanf(TEXT("%lu"), &nAções);
			_tscanf(TEXT("%lu"), &pAção);
			_tprintf(TEXT("\nRecebi Comando: %s %s %lu %lu com tamanho %zu"), comandoAdmin,nomeEmpresa,nAções,pAção, _tcslen(comandoAdmin));
		}
		//TRATA DO COMANDO LISTAR TODAS AS EMPRESAS
		else if (_tcsicmp(TEXT("listc"), comandoAdmin) == 0) {
			_tprintf(TEXT("\nRecebi Comando: %s com tamanho %zu"), comandoAdmin, _tcslen(comandoAdmin));
		}
		//TRATA DO COMANDO REDEFINIR CUSTO DAS AÇÕES DE UMA EMPRESA
		else if (_tcsicmp(TEXT("stock"), comandoAdmin) == 0) {
			_tscanf(TEXT("%s"), &nomeEmpresa);
			_tscanf(TEXT("%lu"), &pAção);
			_tprintf(TEXT("\nRecebi Comando: %s %s %lu com tamanho %zu"), comandoAdmin, nomeEmpresa, pAção, _tcslen(comandoAdmin));
		}
		//TRATA DO COMANDO LISTAR UTLIZADORES
		else if (_tcsicmp(TEXT("users"), comandoAdmin) == 0) {
			_tprintf(TEXT("\nRecebi Comando: %s com tamanho %zu"), comandoAdmin, _tcslen(comandoAdmin));
		}
		//TRATA DO COMANDO PAUSAR AS OPERAÇÕES DE COMPRA E VENDA
		else if (_tcsicmp(TEXT("pause"), comandoAdmin) == 0) {
			_tscanf(TEXT("%lu"), &nSegundos);
			_tprintf(TEXT("\nRecebi Comando: %s %lu com tamanho %zu"), comandoAdmin, nSegundos, _tcslen(comandoAdmin));

		}
		//TRATA DA FALHA DO COMANDO
		else {
			if((_tcsicmp(TEXT("close"), comandoAdmin) != 0))
			_tprintf(TEXT("\nComando: %s introduzido com tamanho %zu, não existe!"), comandoAdmin, _tcslen(comandoAdmin));
		}

	}
	CloseHandle(hPipe);
	RegCloseKey(hChaveClientes);
	return 0;
}

HKEY trataRegedit() {
	HKEY hChaveClientes;
	DWORD nClientes = 5;
	//cria a chave
	if (RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Trabalho_Pratico2024"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hChaveClientes, NULL) != ERROR_SUCCESS) {
		_tprintf(TEXT("Erro ao criar/ abrir chave (%d)\n"), GetLastError());
		return -1;
	}
	//Criar a variável NCLIENTES
	if (RegSetValueEx(hChaveClientes, TEXT("NCLIENTES"), NULL, REG_DWORD, (LPBYTE*)&nClientes, sizeof(DWORD)) != ERROR_SUCCESS) {
		_tprintf(TEXT("Erro ao criar/ abrir variavel (%d)\n"), GetLastError());
		return -1;
	}
	return hChaveClientes;
}

DWORD WINAPI trataComandosClientes(LPVOID hPipe) {
	
	clienteData cd;
	TCHAR buf[200];
	DWORD nbytes;
	_tprintf(TEXT("\À espera de clientes"));
	ConnectNamedPipe(hPipe, NULL);
	_tprintf(TEXT("\nRecebi cliente"));
	
	while ((_tcsicmp(TEXT("exit"), cd.comando)) != 0)
	ReadFile(hPipe, cd.comando, 200 * sizeof(TCHAR), &nbytes, NULL);
	//TRATA DO COMANDO LISTAR TODAS AS EMPRESAS
	if (_tcsicmp(TEXT("listc"), cd.comando) == 0)
	{
		_tprintf(TEXT("\nRecebi do cliente o Comando: %s com tamanho %d"), cd.comando, _tcslen(cd.comando) - 1);
	}
	//TRATA DO COMANDO COMPRAR AÇÕES
	else if (_tcsicmp(TEXT("buy"), cd.comando) == 0) {
		_tprintf(TEXT("\nRecebi do cliente o Comando: %s com tamanho %d"), cd.comando, _tcslen(cd.comando) - 1);
	}
	//TRATA DO COMANDO VENDER AÇÕES
	else if (_tcsicmp(TEXT("sell"), cd.comando) == 0) {
		_tprintf(TEXT("\nRecebi do cliente o Comando: %s com tamanho %d"), cd.comando, _tcslen(cd.comando) - 1);
	}
	//TRATA DO COMANDO BALANCE
	else if (_tcsicmp(TEXT("balance"), cd.comando) == 0) {
		_tprintf(TEXT("\nRecebi do cliente o Comando: %s com tamanho %d"), cd.comando, _tcslen(cd.comando) - 1);
	}
	//TRATA DA FALHA DO COMANDO
	else {
		_tprintf(TEXT("\nComando do cliente: %s introduzido com tamanho %d, não existe!"), cd.comando, _tcslen(cd.comando) - 1);
	}
	
	return 0;
}

DWORD WINAPI verificaClientes(LPVOID hPipe) {
	
	FILE* fp;
	clienteData cd;
	BOOL resultado;
	DWORD nBytes;
	while (1)
	{
		ConnectNamedPipe(hPipe, INFINITE);
		resultado = ReadFile(
			hPipe,
			&cd,
			sizeof(cd),
			&nBytes,
			NULL
		);
		_tprintf(TEXT("Recebi login: %zu, password: %zu"), cd.login, cd.password);
	}

	fp = fopen(UTILIZADORES_REGISTADOS, "r");
	//fread(fp,);

	fclose(fp);
}