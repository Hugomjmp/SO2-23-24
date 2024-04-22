#include "..\\Trabalho_Pr�tico2024\utils.h"

int _tmain(int argc, TCHAR* argv[])
{
	//HANDLES...
	
	HANDLE hPipe, hThreadArray[5];

	//Vari�veis
	DWORD nA��es, pA��o = 0, nSegundos = 0;
	TCHAR comandoAdmin[200], nomeEmpresa[50];
	

	
#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	//_setmode(_fileno(stderr), _O_WTEXT);
#endif


	CriaRegedit(); //TRATA DA VARIAVEL NCLIENTES
	//leRegedit();	//fun��o para tirar depois
	escreveRegedit();//fun��o para tirar depois

	//CIRAR A THREAD PARA TRATAR OS COMANDOS DOS CLIENTES
	/*hThreadArray[0] = NULL; // � bom inicializar a zero para depois podermos testar se a thread foi criada com sucesso
	hThreadArray[0] = CreateThread(
						NULL,					// default security attributes
						0,						// use default stack size
						trataComandosClientes,	// thread function name
						&hPipe,					// argument to thread function
						0,						// use default creation flags
						NULL);
						*/
	hThreadArray[1] = NULL; // � bom inicializar a zero para depois podermos testar se a thread foi criada com sucesso
	hThreadArray[1] = CreateThread(
		NULL,					// default security attributes
		0,						// use default stack size
		verificaClientes,	// thread function name
		NULL,					// argument to thread function
		0,						// use default creation flags
		NULL);
	//COMANDO DO ADMINISTRADOR
	BOOL resultado;
	clienteData cd;
	DWORD nBytes;
	while ((_tcsicmp(TEXT("close"), comandoAdmin)) != 0)
	{
		_tprintf(TEXT("\nComando: "));
		fflush(stdin);
		_tscanf(TEXT("%s"), comandoAdmin);
		_tprintf(TEXT("\nLi -> Comando: %s com tamanho %zu"), comandoAdmin, _tcslen(comandoAdmin));

		//TRATA DO COMANNDO ACRESCENTAR UM EMPRESA
		if (_tcsicmp(TEXT("addc"),comandoAdmin,_tcslen(TEXT("addc"))) == 0)
		{
			_tscanf(TEXT("%s"), &nomeEmpresa);
			_tscanf(TEXT("%lu"), &nA��es);
			_tscanf(TEXT("%lu"), &pA��o);
			_tprintf(TEXT("\nRecebi Comando: %s %s %lu %lu com tamanho %zu"), comandoAdmin,nomeEmpresa,nA��es,pA��o, _tcslen(comandoAdmin));
		}
		//TRATA DO COMANDO LISTAR TODAS AS EMPRESAS
		else if (_tcsicmp(TEXT("listc"), comandoAdmin) == 0) {
			_tprintf(TEXT("\nRecebi Comando: %s com tamanho %zu"), comandoAdmin, _tcslen(comandoAdmin));
		}
		//TRATA DO COMANDO REDEFINIR CUSTO DAS A��ES DE UMA EMPRESA
		else if (_tcsicmp(TEXT("stock"), comandoAdmin) == 0) {
			_tscanf(TEXT("%s"), &nomeEmpresa);
			_tscanf(TEXT("%lu"), &pA��o);
			_tprintf(TEXT("\nRecebi Comando: %s %s %lu com tamanho %zu"), comandoAdmin, nomeEmpresa, pA��o, _tcslen(comandoAdmin));
		}
		//TRATA DO COMANDO LISTAR UTLIZADORES
		else if (_tcsicmp(TEXT("users"), comandoAdmin) == 0) {
			_tprintf(TEXT("\nRecebi Comando: %s com tamanho %zu"), comandoAdmin, _tcslen(comandoAdmin));
		}
		//TRATA DO COMANDO PAUSAR AS OPERA��ES DE COMPRA E VENDA
		else if (_tcsicmp(TEXT("pause"), comandoAdmin) == 0) {
			_tscanf(TEXT("%lu"), &nSegundos);
			_tprintf(TEXT("\nRecebi Comando: %s %lu com tamanho %zu"), comandoAdmin, nSegundos, _tcslen(comandoAdmin));

		}
		//TRATA DA FALHA DO COMANDO
		else {
			if((_tcsicmp(TEXT("close"), comandoAdmin) != 0))
			_tprintf(TEXT("\nComando: %s introduzido com tamanho %zu, n�o existe!"), comandoAdmin, _tcslen(comandoAdmin));
		}

	}
	
	
	return 0;
}

void CriaRegedit() {
	BOOL aux;
	HKEY hChaveClientes;
	DWORD nClientes = 5;

	//cria a chave
	if (RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Trabalho_Pratico2024"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hChaveClientes, NULL) != ERROR_SUCCESS) {
		_tprintf(TEXT("Erro ao criar/ abrir chave (%d)\n"), GetLastError());
		return -1;
	}
	//Criar a vari�vel NCLIENTES

	if (RegSetValueEx(hChaveClientes, TEXT("NCLIENTES"), NULL, REG_DWORD, (LPBYTE*)&nClientes, sizeof(DWORD)) != ERROR_SUCCESS) {
		_tprintf(TEXT("Erro ao criar/ abrir variavel (%d)\n"), GetLastError());
		return -1;
	}
	RegCloseKey(hChaveClientes);
}

DWORD leRegedit() {
	HKEY hChaveClientes;
	LSTATUS resultado;
	DWORD valor;
	DWORD tipo = REG_DWORD;
	DWORD tam = sizeof(DWORD);

	resultado = RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Trabalho_Pratico2024"), 0, KEY_READ, &hChaveClientes); //abre a chave
	if (resultado != ERROR_SUCCESS)
	{
		_tprintf(TEXT("Erro ao abrir a chave (%d)\n"), GetLastError());
		return -1;
	}
	resultado = RegQueryValueEx(hChaveClientes,TEXT("NCLIENTES"),NULL,&tipo,(LPBYTE)&valor, &tam);//le o conteudo
	if (resultado != ERROR_SUCCESS) {
		_tprintf(TEXT("Erro ao ler o nome (%d)\n"), GetLastError());
		return -1;
	}
	_tprintf(TEXT("NCLIENTES (%lu)\n"), valor);
	return valor;
}

void escreveRegedit() {
	HKEY hChaveClientes;
	LSTATUS resultado;
	DWORD tipo = REG_DWORD;
	DWORD tam = sizeof(DWORD);
	DWORD valor;
	valor = leRegedit();
	valor--;
	resultado = RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Trabalho_Pratico2024"), 0, KEY_SET_VALUE, &hChaveClientes); //abre a chave
	if (resultado != ERROR_SUCCESS)
	{
		_tprintf(TEXT("Erro ao abrir a chave (%d)\n"), GetLastError());
		return -1;
	}
	resultado = RegSetValueEx(hChaveClientes, TEXT("NCLIENTES"),0,REG_DWORD,(LPBYTE)&valor,tam);//le o conteudo
	if (resultado != ERROR_SUCCESS) {
		_tprintf(TEXT("Erro ao escrever o valor (%d)\n"), GetLastError());
		return -1;
	}
	
}
// Por acabar...
DWORD WINAPI trataComandosClientes() {
	
	clienteData cd;
	TCHAR buf[200];
	DWORD nbytes;
	HANDLE hPipe;
	_tprintf(TEXT("\n� espera de clientes"));
	//ConnectNamedPipe(hPipe, NULL);
	_tprintf(TEXT("\nRecebi cliente"));
	
	while ((_tcsicmp(TEXT("exit"), cd.comando)) != 0)
	//ReadFile(hPipe, cd.comando, 200 * sizeof(TCHAR), &nbytes, NULL);
	//TRATA DO COMANDO LISTAR TODAS AS EMPRESAS
	if (_tcsicmp(TEXT("listc"), cd.comando) == 0)
	{
		_tprintf(TEXT("\nRecebi do cliente o Comando: %s com tamanho %d"), cd.comando, _tcslen(cd.comando) - 1);
	}
	//TRATA DO COMANDO COMPRAR A��ES
	else if (_tcsicmp(TEXT("buy"), cd.comando) == 0) {
		_tprintf(TEXT("\nRecebi do cliente o Comando: %s com tamanho %d"), cd.comando, _tcslen(cd.comando) - 1);
	}
	//TRATA DO COMANDO VENDER A��ES
	else if (_tcsicmp(TEXT("sell"), cd.comando) == 0) {
		_tprintf(TEXT("\nRecebi do cliente o Comando: %s com tamanho %d"), cd.comando, _tcslen(cd.comando) - 1);
	}
	//TRATA DO COMANDO BALANCE
	else if (_tcsicmp(TEXT("balance"), cd.comando) == 0) {
		_tprintf(TEXT("\nRecebi do cliente o Comando: %s com tamanho %d"), cd.comando, _tcslen(cd.comando) - 1);
	}
	//TRATA DA FALHA DO COMANDO
	else {
		_tprintf(TEXT("\nComando do cliente: %s introduzido com tamanho %d, n�o existe!"), cd.comando, _tcslen(cd.comando) - 1);
	}
	
	return 0;
}
//-------
DWORD WINAPI verificaClientes() {
	
	FILE* fp;
	clienteData cd;
	BOOL resultado;
	DWORD nBytes;
	TCHAR string[100];
	HANDLE hSemClientes;
	HANDLE hPipe;

	//CRIAR O NAMEDPIPE BOLSA PARA INTERA��O DOS CLIENTES
	hPipe = CreateNamedPipe(
		NOME,	// Nome do pipe
		PIPE_ACCESS_DUPLEX,			// acesso em modo de escrita e de leitura
		PIPE_TYPE_MESSAGE |			// message type pipe 
		PIPE_READMODE_MESSAGE |		// message-read mode 
		PIPE_WAIT,					// blocking mode 
		PIPE_UNLIMITED_INSTANCES,							// max. instances  
		sizeof(clienteData),					// output buffer size 
		sizeof(clienteData),					// input buffer size 
		0,							// client time-out 
		NULL);						// default security attribute 
	if (hPipe != INVALID_HANDLE_VALUE)
	{
		_tprintf(TEXT("[ERRO] Falhou ao criar CreateNamedPipe, %d.\n"), GetLastError());
		return -1;
	}
	_tprintf(TEXT("[ERRO] antes de esperar.\n"));
	resultado = ConnectNamedPipe(hPipe, INFINITE);
		if (resultado);
	_tprintf(TEXT("[ERRO] depois de esperar.\n"));
	/*
	while (1){
	//ReadFile(hPipe, &receivedInfo, sizeof(LoginInfo), &bytesRead, NULL)
	resultado = ReadFile(
		hPipe,
		&cd,
		sizeof(clienteData),
		&nBytes,
		NULL
	);
	if (resultado != ERROR_SUCCESS) {
		_tprintf(TEXT("[ERRO] Falhou ao ler o NamedPipe, %d.\n"), GetLastError());
		return -1;
	}
	_tprintf(TEXT("Recebi login: %zu, password: %zu"), cd.login, cd.password);

}*/















	/*
	hSemClientes = CreateSemaphoreA(
		NULL,
		5,
		5,
		TEXT("semaforoClientes")
		);
		if (hSemClientes == NULL) {
			_tprintf(TEXT("[ERRO] ao criar o sem�foro: %d\n"),GetLastError());
			return -1;
		}
		*/
	














	fp = fopen(UTILIZADORES_REGISTADOS, "r"); //abertura do ficheiro Clientes.txt para leitura
	if (fp == NULL) //verifica se o ficheiro existe
	{
		_tprintf(TEXT("[ERRO] ao abrir o ficheiro! \n"));
		return -1;
	}



	while (fgetws(string, 100, fp)) {
		_tprintf(TEXT("%s \n"), string);
	}



	//CloseHandle(hSemClientes);
	CloseHandle(hPipe);
	fclose(fp);
}