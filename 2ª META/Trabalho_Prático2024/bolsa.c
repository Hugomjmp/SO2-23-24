#include "..\\Trabalho_Prático2024\utils.h"

int _tmain(int argc, TCHAR* argv[])
{
	//HANDLES...

	HANDLE hPipe, hThreadArray[5] = {NULL,NULL,NULL,NULL,NULL},
		   hEvent, hMapFile, hMutex, hMutexRead, hEventRead, hSemClientes;

	//Variáveis
	DWORD nSegundos = 0, nAções = 0, linha, nBytes, numeros, contador = 0;
	DWORD digitos = 0;
	DWORD NCLIENTES = leRegedit();
	float pAção;
	TCHAR comandoAdmin[200], nomeEmpresa[50], string[100];
	BOOL resultado;
	
	
	//ficheiros
	FILE* fp,*fpU;

	//estruturas
	clienteData cliData;
	userData users[5];
	empresaData empresas[30];
	empresaData* emP;
	ControlData ctrlData = {.empresas = empresas, .users = users};
	

	//ControlData* cdata;
	
#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif

	//###############################################################
	//#																#
	//#					INICIALIZAR MATRIZ USER						#
	//#																#
	//###############################################################
	for (int i = 0; i < 5; i++) {
			_tcscpy(users[i].username, TEXT("-1"));
			_tcscpy(users[i].password, TEXT("-2"));
			users[i].saldo = 0;
			users[i].estado = 0;
		
	}
	//---------------------------------------------------------------
	fpU = fopen(UTILIZADORES_REGISTADOS, "r"); //abertura do ficheiro Clientes.txt para leitura
	if (fpU == NULL) //verifica se o ficheiro existe
	{
		_tprintf(TEXT("[ERRO] ao abrir o ficheiro! \n"));
		return -1;
	}
	linha = 0;
	while (linha < 5 && fwscanf(fpU, TEXT("%s %s %f"), users[linha].username, users[linha].password, &users[linha].saldo) == 3) {
		linha++;
	}
	//###############################################################
	//#																#
	//#				INICIALIZAR MATRIZ empresas						#
	//#				com -1, 0, 0.0 em todas as linhas				#
	//###############################################################
	for (DWORD i = 0; i < MAX_EMPRESAS; i++)
	{
		_tcscpy(empresas[i].nomeEmpresa, TEXT("-1"));
		empresas[i].nAções = 0;
		empresas[i].pAção = 0.0;
	}
	//---------------------------------------------------------------
	CriaRegedit(); //TRATA DA VARIAVEL NCLIENTES
	
	//escreveRegedit();//função para tirar depois


	//###############################################################
	//#																#
	//#						Eventos			 						#
	//#																#
	//###############################################################
	//falta os if's

	hEvent = CreateEvent(
		NULL,			//lpEventAttributes
		TRUE,			//bManualReset
		FALSE,			//bInitialState
		EVENT_NAME		//lpName
	);
	hEventRead = CreateEvent(
		NULL,			//lpEventAttributes
		TRUE,			//bManualReset
		FALSE,			//bInitialState
		EVENT_NAME_O	//lpName
	);
	//---------------------------------------------------------------
	//###############################################################
	//#																#
	//#						Semaforos		 						#
	//#																#
	//###############################################################

	hSemClientes = CreateSemaphore(
		NULL,
		0,
		NCLIENTES,
		SEM_CLIENT_NAME);
	if (hSemClientes == NULL) {
		_tprintf(TEXT("[ERRO] Falhou ao criar o semáforo: %d\n"), GetLastError());
		return -1;
	}






	//---------------------------------------------------------------
	//###############################################################
	//#																#
	//#						Shared Memory	 						#
	//#																#
	//###############################################################
	
	//CRIA shared memory
	
	hMapFile = CreateFileMapping(
			INVALID_HANDLE_VALUE,	// Ficheiro a usar
			NULL,					// LPSECURITY_ATTRIBUTES lpAttributes,
			PAGE_READWRITE,			// flags para: escrita/leitura/execução
			0,						// Tamanho dado em duas DWORDS
			sizeof(empresaData),		// (mais significativo e menos significativo)
			SHM_NAME				// Nome a dar ao recurso (fich. mapeado)
	);				
	if (hMapFile == NULL) {
		_tprintf(TEXT("Error: CreateFileMapping (%d)\n"), GetLastError());
		return 1;
	}
	
	emP = (empresaData*)MapViewOfFile(
			hMapFile,				// Handle do ficheiro mapeado
			FILE_MAP_ALL_ACCESS,	// Flags de acesso (ler, escrever)
			0,						// Início dentro do bloco pretendido
			0,						// dentro do ficheiro (+signific., -signific.)
			sizeof(empresaData)		// Tamanho da view pretendida
	);
	if (emP == NULL) {
		_tprintf(TEXT("ERROR: MapViewOfFile (%d)\n"), GetLastError());
		CloseHandle(hMapFile);
		return 1;
	}
	for (DWORD i = 0; i < MAX_EMPRESAS; i++)
	{
		_tcscpy(emP[i].nomeEmpresa, TEXT("-1"));
		emP[i].nAções = 0;
		emP[i].pAção = 0.0;
	}
	//---------------------------------------------------------------
	//###############################################################
	//#																#
	//#							MUTEX								#
	//#																#
	//###############################################################

	hMutex = CreateMutex(
		NULL,
		FALSE,
		MUTEX_NAME
	);

	if (hMutex == NULL) {
		_tprintf(TEXT("ERROR: Mutex (%d)\n"), GetLastError());
		return 1;
	}
	hMutexRead = CreateMutex(
		NULL,
		FALSE,
		MUTEX_NAME_O
	);

	if (hMutexRead == NULL) {
		_tprintf(TEXT("ERROR: Mutex (%d)\n"), GetLastError());
		return 1;
	}

	//---------------------------------------------------------------



	//###############################################################
	//#																#
	//#							Threads								#
	//#																#
	//###############################################################
	//CIRAR A THREAD PARA TRATAR OS COMANDOS DOS CLIENTES
	hThreadArray[1] = NULL; // é bom inicializar a zero para depois podermos testar se a thread foi criada com sucesso
	hThreadArray[1] = CreateThread(
		NULL,					// default security attributes
		0,						// use default stack size
		verificaClientes,	// thread function name
		&ctrlData,					// argument to thread function
		0,						// use default creation flags
		NULL);
	if (hThreadArray[1] == NULL) {
		_tprintf(TEXT("Erro a criar a thread. Código de erro: %d\n", GetLastError()));
		return 1;
	}
	hThreadArray[2] = NULL; // é bom inicializar a zero para depois podermos testar se a thread foi criada com sucesso
	hThreadArray[2] = CreateThread(
		NULL,					// default security attributes
		0,						// use default stack size
		variaPreços,			// thread function name
		empresas,				// argument to thread function
		0,						// use default creation flags
		NULL);
	if (hThreadArray[2] == NULL) {
		_tprintf(TEXT("Erro a criar a thread. Código de erro: %d\n", GetLastError()));
		return 1;
	}
	hThreadArray[3] = NULL; // é bom inicializar a zero para depois podermos testar se a thread foi criada com sucesso
	hThreadArray[3] = CreateThread(
		NULL,					// default security attributes
		0,						// use default stack size
		SMtoLocal,				// thread function name
		empresas,				// argument to thread function
		0,						// use default creation flags
		NULL);
	if (hThreadArray[3] == NULL) {
		_tprintf(TEXT("Erro a criar a thread. Código de erro: %d\n", GetLastError()));
		return 1;
	}
	//---------------------------------------------------------------
	
	//###############################################################
	//#																#
	//#				Processa Comando de Administrador				#
	//#																#
	//###############################################################
	_tprintf(TEXT("\t\t\t#################################################################\n"));
	_tprintf(TEXT("\t\t\t#\t\t\t\t\t\t\t\t#\n"));
	_tprintf(TEXT("\t\t\t#\t\t\tBOLSA DE VALORES SERVER\t\t\t#\n"));
	_tprintf(TEXT("\t\t\t#\t\t\t\t\t\t\t\t#\n"));
	_tprintf(TEXT("\t\t\t#################################################################\n"));
	while ((_tcsicmp(TEXT("close"), comandoAdmin)) != 0)
	{
		_tprintf(TEXT("\n\t\t\t-----------------------------------------------------------------"));
		_tprintf(TEXT("\n\t\t\t|\t\t\t\tMENU\t\t\t\t|"));
		_tprintf(TEXT("\n\t\t\t-----------------------------------------------------------------"));
		_tprintf(TEXT("\n\t\t\t| addc - Acrescentar uma empresa \t\t\t\t|"));
		_tprintf(TEXT("\n\t\t\t| adde - Acrescentar empresas via Empresas.txt \t\t\t|"));
		_tprintf(TEXT("\n\t\t\t| listc - Listar todas as empresas \t\t\t\t|"));
		_tprintf(TEXT("\n\t\t\t| stock - Redefinir custo das ações de uma empresa \t\t|"));
		_tprintf(TEXT("\n\t\t\t| users - Listar utilizadores \t\t\t\t\t|"));
		_tprintf(TEXT("\n\t\t\t| pause - Pausar as operações de compra e venda \t\t|"));
		_tprintf(TEXT("\n\t\t\t| clear - Limpa o ecrã \t\t\t\t\t\t|"));
		_tprintf(TEXT("\n\t\t\t| close - Encerrar a plataforma \t\t\t\t|"));
		_tprintf(TEXT("\n\t\t\t-----------------------------------------------------------------"));
		_tprintf(TEXT("\n\t\t\tComando: "));
		fflush(stdin);
		_tscanf(TEXT("%s"), comandoAdmin);
		//_tprintf(TEXT("\nLi -> Comando: %s com tamanho %zu"), comandoAdmin, _tcslen(comandoAdmin));

		//TRATA DO COMANNDO ACRESCENTAR UM EMPRESA
		if (_tcsicmp(TEXT("addc"), comandoAdmin, _tcslen(TEXT("addc"))) == 0)
		{
			_tscanf(TEXT("%s"), &nomeEmpresa);
			_tscanf(TEXT("%lu"), &nAções);
			_tscanf(TEXT("%f"), &pAção);
			DWORD empresaAdiciona = 1;
			for (DWORD i = 0; i < MAX_EMPRESAS; i++) {
				if (empresaAdiciona == 1) {
					if ((_tcsicmp(TEXT("-1"), empresas[i].nomeEmpresa)) == 0) {

						//evento
						SetEvent(hEvent);
						//bloqueia no mutex
						WaitForSingleObject(hMutex, INFINITE);
						_tcscpy(empresas[i].nomeEmpresa, nomeEmpresa);
						empresas[i].nAções = nAções;
						empresas[i].pAção = pAção;
						empresaAdiciona = 0;
						//copia dados para a sharedmemory
						CopyMemory(emP, empresas, sizeof(empresas));
						//relase ao mutex
						ReleaseMutex(hMutex);
						//resetEvento
						ResetEvent(hEvent);

					}
				}
			}
		}
		//TRATA DO COMANNDO ACRESCENTAR UM EMPRESA via file txt
		if (_tcsicmp(TEXT("adde"), comandoAdmin, _tcslen(TEXT("adde"))) == 0)
		{
			fp = fopen(EMPRESAS, "r"); //abertura do ficheiro Clientes.txt para leitura
			if (fp == NULL) //verifica se o ficheiro existe
			{
				_tprintf(TEXT("[ERRO] ao abrir o ficheiro! \n"));
				return -1;
			}
			linha = 0;
			while (linha < MAX_EMPRESAS && fwscanf(fp, TEXT("%s %lu %f"), &empresas[linha].nomeEmpresa, &empresas[linha].nAções, &empresas[linha].pAção) == 3) {
				linha++;
			}

			//bloqueia no mutex
			WaitForSingleObject(hMutex, INFINITE);
			//copia dados para a sharedmemory
			CopyMemory(emP, empresas, sizeof(empresas));
			//relase ao mutex
			ReleaseMutex(hMutex);
			//evento
			SetEvent(hEvent);
			//resetEvento
			ResetEvent(hEvent);
			

			
			
			fclose(fp);
		}
		//TRATA DO COMANDO LISTAR TODAS AS EMPRESAS
		else if (_tcsicmp(TEXT("listc"), comandoAdmin) == 0) {
			_tprintf(TEXT("\n\t\t| ID | |\t NOME\t\t| |\t Num_Ações\t| |\t Preço-Ação\t|\n"));
			_tprintf(TEXT("\t\t---------------------------------------------------------------------------------\n"));
			for (DWORD i = 0; i < MAX_EMPRESAS; i++) { //conta quantas empresas estão na tabela
				if (_tcsicmp(TEXT("-1"), empresas[i].nomeEmpresa) != 0)
				{
					contador++;
				}

			}
			for (DWORD i = 0; i < contador; i++)
			{
				if (i < 9) { //corrige o espaçamento do ID
					_tprintf(TEXT("\t\t| %d  |"), i + 1);
				}else{
					_tprintf(TEXT("\t\t| %d |"), i + 1);
				}
					if (_tcslen(empresas[i].nomeEmpresa) <= 5)
					{
						if (_tcscmp(empresas[i].nomeEmpresa, TEXT("-1")) == 0) {
							_tprintf(TEXT(" |\t   \t\t|"), empresas[i].nomeEmpresa); // coloca esppaços vazios onde está "-1"
						}else
							_tprintf(TEXT(" |\t %s \t\t|"), empresas[i].nomeEmpresa);
					}
					else {
						_tprintf(TEXT(" |\t %s \t|"), empresas[i].nomeEmpresa);

					}			
					_tprintf(TEXT(" |\t %lu \t\t|"), empresas[i].nAções);
					//contar os digitos para retificar espaçamento do Preço Ação
					digitos = 0;
					numeros = empresas[i].pAção;
					while (numeros >= 1) { 
						numeros /= 10;
						digitos++;
					}
					if (digitos >= 2)
						_tprintf(TEXT(" |\t %.2f€ \t|"), empresas[i].pAção);
					else
						_tprintf(TEXT(" |\t %.2f€ \t\t|"), empresas[i].pAção);

				_tprintf(TEXT("\n\t\t---------------------------------------------------------------------------------\n"));
			}
			contador = 0;
		}
		//TRATA DO COMANDO REDEFINIR CUSTO DAS AÇÕES DE UMA EMPRESA
		else if (_tcsicmp(TEXT("stock"), comandoAdmin) == 0) {
			_tscanf(TEXT("%s"), &nomeEmpresa);
			_tscanf(TEXT("%f"), &pAção);

			for (DWORD i = 0; i < MAX_EMPRESAS; i++) {
					if ((_tcsicmp(empresas[i].nomeEmpresa, nomeEmpresa)) == 0) {
						//evento
						SetEvent(hEvent);
						//bloqueia no mutex
						WaitForSingleObject(hMutex, INFINITE);
						empresas[i].pAção = pAção;
						//copia dados para a sharedmemory
						CopyMemory(emP, empresas, sizeof(empresas));
						//relase ao mutex
						ReleaseMutex(hMutex);
						//resetEvento
						ResetEvent(hEvent);
					}
			}
		}
		//TRATA DO COMANDO LISTAR UTLIZADORES
		else if (_tcsicmp(TEXT("users"), comandoAdmin) == 0) {
			_tprintf(TEXT("\n\t\t| ID | |\t NOME\t\t| |\t Saldo \t\t| |\t Estado\t\t|\n"));
			_tprintf(TEXT("\t\t---------------------------------------------------------------------------------\n"));
			for (DWORD i = 0; i < 5; i++)
			{
				_tprintf(TEXT("\t\t| %d  |"), i + 1);

				_tprintf(TEXT(" |\t %s \t\t|"), users[i].username);
				//_tprintf(TEXT(" |\t %.2f \t\t|"), user[i][2].saldo);
				digitos = 0;
				numeros = users[i].saldo;
				while (numeros >= 1) {
					numeros /= 10;
					digitos++;
				}
				
				if (digitos >= 3)
					_tprintf(TEXT(" |\t %.2f€ \t|"), users[i].saldo);
				else
					_tprintf(TEXT(" |\t %.2f€ \t|"), users[i].saldo);
				if(users[i].estado == 1)
					_tprintf(TEXT(" |\t ONLINE \t|"));
				else
					_tprintf(TEXT(" |\t OFFLINE \t|"));
				
				_tprintf(TEXT("\n\t\t---------------------------------------------------------------------------------\n"));
			}
		}

		//TRATA DO COMANDO PAUSAR AS OPERAÇÕES DE COMPRA E VENDA
		else if (_tcsicmp(TEXT("pause"), comandoAdmin) == 0) {
			_tscanf(TEXT("%lu"), &nSegundos);

			Sleep(nSegundos * 1000); //em segundos
			_tprintf(TEXT("\nRecebi Comando: %s %lu com tamanho %zu"), comandoAdmin, nSegundos, _tcslen(comandoAdmin));

		}
		else if (_tcsicmp(TEXT("clear"), comandoAdmin) == 0) { //limpa o ecra
			system("cls");
			_tprintf(TEXT("\t\t\t#################################################################\n"));
			_tprintf(TEXT("\t\t\t#\t\t\t\t\t\t\t\t#\n"));
			_tprintf(TEXT("\t\t\t#\t\t\tBOLSA DE VALORES SERVER\t\t\t#\n"));
			_tprintf(TEXT("\t\t\t#\t\t\t\t\t\t\t\t#\n"));
			_tprintf(TEXT("\t\t\t#################################################################\n"));

		}
		//TRATA DA FALHA DO COMANDO
		else {
			if((_tcsicmp(TEXT("close"), comandoAdmin) != 0))
			_tprintf(TEXT("\nComando: %s introduzido com tamanho %zu, não existe!"), comandoAdmin, _tcslen(comandoAdmin));
		}

	}
	for (DWORD i = 0; i < 5; i++)
		{
			CloseHandle(hThreadArray[i]);
		}
	CloseHandle(hMutex);
	CloseHandle(hEvent);
	UnmapViewOfFile(emP);
	CloseHandle(hMapFile);
	fclose(fpU);
	return 0;
}
//###############################################################
//#																#
//#				Cria a chave no Regedit							#
//#																#
//###############################################################
void CriaRegedit() {
	BOOL aux;
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
	RegCloseKey(hChaveClientes);
}
//---------------------------------------------------------------
//###############################################################
//#																#
//#				Lê o valor da chave no Regedit					#
//#																#
//###############################################################
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
	//_tprintf(TEXT("NCLIENTES (%lu)\n"), valor); //debug...
	return valor;
}
//---------------------------------------------------------------
//###############################################################
//#																#
//#				Escreve o valor da chave no Regedit				#
//#																#
//###############################################################
void escreveRegedit() {
	HKEY hChaveClientes;
	LSTATUS resultado;
	DWORD tipo = REG_DWORD;
	DWORD tam = sizeof(DWORD);
	DWORD valor;
	valor = leRegedit();
	valor--;
	resultado = RegOpenKeyEx(
			HKEY_CURRENT_USER, 
			TEXT("Software\\Trabalho_Pratico2024"), 
			0, 
			KEY_SET_VALUE, 
			&hChaveClientes
	); //abre a chave
	if (resultado != ERROR_SUCCESS)
	{
		_tprintf(TEXT("Erro ao abrir a chave (%d)\n"), GetLastError());
		return -1;
	}
	resultado = RegSetValueEx(
			hChaveClientes, 
			TEXT("NCLIENTES"),
			0,
			REG_DWORD,
			(LPBYTE)&valor,tam
	);//le o conteudo
	if (resultado != ERROR_SUCCESS) {
		_tprintf(TEXT("Erro ao escrever o valor (%d)\n"), GetLastError());
		return -1;
	}
	
}
//---------------------------------------------------------------
// Por acabar...
DWORD WINAPI trataComandosClientes(LPVOID ctrlData) {
	ControlData* cdata = (ControlData*)ctrlData;
	clienteData cliente;
	DWORD nBytes;
	HANDLE writeResult, hPipe;


	hPipe = CreateFile(
		NAME_PIPE,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (hPipe == NULL) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'! (CreateFile)\n"), NAME_PIPE);
		return -1;
	}


	while ((_tcsicmp(TEXT("exit"), cliente.comando)) != 0){
		if (!ReadFile(
			hPipe,
			&cliente,
			sizeof(clienteData),
			&nBytes,
			NULL))
		{
			_tprintf(TEXT("\nErro ao ler do pipe do cliente: %d"), GetLastError());
			break;
		}
		//ReadFile(hPipe, cd.comando, 200 * sizeof(TCHAR), &nbytes, NULL);
		//TRATA DO COMANDO LISTAR TODAS AS EMPRESAS
		if (_tcsicmp(TEXT("listc"), cliente.comando) == 0)
		{
			writeResult = WriteFile(
				hPipe,
				&cliente,
				sizeof(clienteData),
				&nBytes,
				NULL
			);
			FlushFileBuffers(cdata->hPipe[0]);
			_tprintf(TEXT("\nRecebi do cliente o Comando: %s com tamanho %d"), cliente.comando, _tcslen(cliente.comando) - 1);
		}
		//TRATA DO COMANDO COMPRAR AÇÕES
		else if (_tcsicmp(TEXT("buy"), cliente.comando) == 0) {
			writeResult = WriteFile(
				hPipe,
				&cliente,
				sizeof(clienteData),
				&nBytes,
				NULL
			);
			FlushFileBuffers(cdata->hPipe[0]);
			_tprintf(TEXT("\nRecebi do cliente o Comando: %s com tamanho %d"), cliente.comando, _tcslen(cliente.comando) - 1);
		}
		//TRATA DO COMANDO VENDER AÇÕES
		else if (_tcsicmp(TEXT("sell"), cliente.comando) == 0) {
			writeResult = WriteFile(
				hPipe,
				&cliente,
				sizeof(clienteData),
				&nBytes,
				NULL
			);
			FlushFileBuffers(cdata->hPipe[0]);
			_tprintf(TEXT("\nRecebi do cliente o Comando: %s com tamanho %d"), cliente.comando, _tcslen(cliente.comando) - 1);
		}
		//TRATA DO COMANDO BALANCE
		else if (_tcsicmp(TEXT("balance"), cliente.comando) == 0) {
			writeResult = WriteFile(
				hPipe,
				&cliente,
				sizeof(clienteData),
				&nBytes,
				NULL
			);
			FlushFileBuffers(hPipe);
			_tprintf(TEXT("\nRecebi do cliente o Comando: %s com tamanho %d"), cliente.comando, _tcslen(cliente.comando) - 1);
		}
		//TRATA DA FALHA DO COMANDO
		else {
			_tprintf(TEXT("\nComando do cliente: %s introduzido com tamanho %d, não existe!"), cliente.comando, _tcslen(cliente.comando) - 1);
		}
	}
	return 0;
}
//-------

//###############################################################
//#																#
//#				Verificao Clientes através de semaforo			#
//#																#
//###############################################################
DWORD WINAPI verificaClientes(LPVOID ctrlData) {
	ControlData* cdata = (ControlData*)ctrlData;
	

	//userData* userArray = (userData*)users;
	FILE* fp;
	clienteData cliData;
	BOOL resultado, readResult, writeResult;
	DWORD nBytes;
	TCHAR string[100];
	HANDLE hSemClientes;
	HANDLE hPipe;
	HANDLE hThreadArray[5];
	DWORD NCLIENTES = leRegedit();
	

	//CRIAR O NAMEDPIPE BOLSA PARA INTERAÇÃO DOS CLIENTES
	hPipe = CreateNamedPipe(
		NAME_PIPE,					// Nome do pipe
		PIPE_ACCESS_DUPLEX,			// acesso em modo de escrita e de leitura
		PIPE_TYPE_MESSAGE |			// message type pipe 
		PIPE_READMODE_MESSAGE |		// message-read mode 
		PIPE_WAIT,					// blocking mode 
		PIPE_UNLIMITED_INSTANCES,	// max. instances  
		sizeof(clienteData),		// output buffer size 
		sizeof(clienteData),		// input buffer size 
		0,							// client time-out 
		NULL);						// default security attribute 
	if (hPipe == INVALID_HANDLE_VALUE)
	{
		_tprintf(TEXT("[ERRO] Falhou ao criar CreateNamedPipe, %d.\n"), GetLastError());
		return -1;
	}

	//SEMAFOROS
	hSemClientes = OpenSemaphore(
		EVENT_MODIFY_STATE,
		FALSE,
		SEM_CLIENT_NAME
	);
	if (hSemClientes == NULL) {
		_tprintf(TEXT("Erro ao abrir o semáforo. Código de erro: %d\n", GetLastError()));
		return 1;
	}
	resultado = ConnectNamedPipe(hPipe, NULL); //espera que o cliente entre
	if (!resultado) {
		_tprintf(TEXT("[ERRO] Falhou ao aguardar conexão do cliente, %d.\n"), GetLastError());
	}
	_tprintf(TEXT("empresa %s.\n"), cdata->empresas[0].nomeEmpresa);
	
	while (1)
	{
		
			resultado = ReadFile( //recebe as credenciais
				hPipe,					// handle to pipe 
				&cliData,				// buffer to receive data 
				sizeof(clienteData),	// size of buffer 
				&nBytes,				// number of bytes read 
				NULL);					// not overlapped I/O 
			_tprintf(TEXT("RECEBI: \n"));
			if (nBytes != 0) {
				_tprintf(TEXT("RECEBI login: %s\n"), cliData.login);
				_tprintf(TEXT("RECEBI password: %s\n"), cliData.password);
				for (DWORD i = 0; i < MAX_CLIENTES; i++)
				{
					_tprintf(TEXT("empresa %s.\n"), cdata->empresas[i].nomeEmpresa);
					_tprintf(TEXT("user %s.\n"), cdata->users[i].username);
					if (_tcsicmp(cdata->users[i].username, cliData.login) == 0 && _tcsicmp(cdata->users[i].password, cliData.password) == 0)
					{
						_tprintf(TEXT("VAGAS: %lu\n"), NCLIENTES);
						if (NCLIENTES > 0)//VERIFICA SE TEM VAGAS...
						{
							cdata->users[i].estado = 1;
							_tcscpy(cliData.RESPOSTA, cdata->users[i].username);

							writeResult = WriteFile(
								hPipe,
								&cliData,
								sizeof(clienteData),
								&nBytes,
								NULL
							);
							FlushFileBuffers(hPipe);
							ReleaseSemaphore(
								hSemClientes,
								1,
								NULL
							);
							//CRIA A THREAD PARA O CLIENTE USAR COMANDOS
							hThreadArray[0] = NULL; // é bom inicializar a zero para depois podermos testar se a thread foi criada com sucesso
							hThreadArray[0] = CreateThread(
								NULL,					// default security attributes
								0,						// use default stack size
								trataComandosClientes,		// thread function name
								&cdata,					// argument to thread function
								0,						// use default creation flags
								NULL);
							if (hThreadArray[0] == NULL) {
								_tprintf(TEXT("Erro a criar a thread. Código de erro: %d\n", GetLastError()));
								return 1;
							}

						}

					}
					else
					{
						_tcscpy(cliData.RESPOSTA, TEXT("FAIL"));
						writeResult = WriteFile(
							hPipe,
							&cliData,
							sizeof(clienteData),
							&nBytes,
							NULL
						);
						FlushFileBuffers(hPipe);
					}
				}
			}
			
	}





		
		
	
	





		//_tprintf(TEXT("aqui1.\n"));
	while (_tcsicmp(TEXT("close"), cliData.comando) != 0)
	{

		//_tprintf(TEXT("aqui2.\n"));
		readResult = ReadFile(
			hPipe,        // handle to pipe 
			&cliData,   // buffer to receive data 
			sizeof(clienteData), // size of buffer 
			&nBytes, // number of bytes read 
			NULL);        // not overlapped I/O 
		if (!readResult)
		{
			//_tprintf(TEXT("RECEBI login: %s\n"), cliData.login);
			//_tprintf(TEXT("RECEBI login: %s\n"), cliData.password);
		}
		if (nBytes != 0) {
			_tprintf(TEXT("RECEBI comando: %s\n"), cliData.comando);
		}

	}
	//_tprintf(TEXT("aqui3.\n"));
	//_tprintf(TEXT("[ERRO] antes de esperar.\n"));

	//_tprintf(TEXT("[INFO] Cliente conectado com sucesso.\n"));
	/*

	

	fp = fopen(UTILIZADORES_REGISTADOS, "r");
	if (fp == NULL) {
		_tprintf(TEXT("[ERRO] Falhou ao abrir o arquivo de clientes registrados.\n"));
		return -1;
	}
	
	while (fgetws(string, 100, fp)) {
		
		WaitForSingleObject(hSemClientes, INFINITE);

		
		_tprintf(TEXT("%s \n"), string);

		
		ReleaseSemaphore(hSemClientes, 1, NULL);
	}
	*/
	//CloseHandle(hSemClientes);
	//fclose(fp);
	CloseHandle(hPipe);

	return 0;
}


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

//###############################################################
//#																#
//#				Variação aleatoria de preços					#
//#																#
//###############################################################
//falta mandar para a SHARED MEMORY!!
DWORD WINAPI variaPreços(LPVOID empresas) {
	//empresaData(*data)[MAX_COLUNA] = (empresaData(*)[MAX_COLUNA])empresas;
	empresaData *empresasArry = (empresaData*)empresas;
	DWORD contador = 0;
	DWORD linhaAleatoria = 0;
	DWORD tipoAlteração = 0;
	while (1)
	{
		for (DWORD i = 0; i < MAX_EMPRESAS; i++) {
				if (_tcsicmp(TEXT("-1"), empresasArry[i].nomeEmpresa) != 0)
				{
					contador++;
				}
			
		}
		if (contador != 0)
		{
			tipoAlteração = rand() % 50;
			if (tipoAlteração < 25)
			{
				linhaAleatoria = rand() % contador;
				empresasArry[linhaAleatoria].pAção = empresasArry[linhaAleatoria].pAção - 
					(empresasArry[linhaAleatoria].pAção * 0.2); //diminui 20%
				
			}
			else
			{
				linhaAleatoria = rand() % contador;
				empresasArry[linhaAleatoria].pAção = empresasArry[linhaAleatoria].pAção + 
					(empresasArry[linhaAleatoria].pAção * 0.2); //aumenta 20%
				
			}

		}

		contador = 0;
		Sleep(10000); //10segundos alterar se for o caso
	}
	return 0;
}

//---------------------------------------------------------------


DWORD WINAPI SMtoLocal(LPVOID empresas) {
	empresaData* empresasArry = (empresaData*)empresas;
	empresaData* emP;
	HANDLE hMapFile, hMutexRead, hEventRead;


	//###############################################################
	//#																#
	//#						Eventos			 						#
	//#																#
	//###############################################################
	hEventRead = OpenEvent(
		SYNCHRONIZE,	//dwDesiredAccess,
		FALSE,			//bInheritHandle,
		EVENT_NAME_O	//lpName
	);
	if (hEventRead == NULL) {
		_tprintf(TEXT("Erro ao abrir o evento. Código de erro: %d\n", GetLastError()));
		return 1;
	}
	//###############################################################
	//#																#
	//#							MUTEX								#
	//#																#
	//###############################################################
	hMutexRead = OpenMutex(
		MUTEX_ALL_ACCESS,
		FALSE,
		MUTEX_NAME_O);

	if (hMutexRead == NULL) {
		_tprintf(TEXT("ERROR: Mutex (%d)\n"), GetLastError());
		return 1;
	}

	hMapFile = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,	// acesso pretendido 
		FALSE,
		SHM_NAME			// nome dado ao recurso (ficheiro mapeado)
	);
	if (hMapFile == NULL) {
		_tprintf(TEXT("Error: OpenFileMapping (%d)\n"), GetLastError());
		return 1;
	}
	emP = (empresaData*)MapViewOfFile(
		hMapFile,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		sizeof(empresaData)
	);
	if (emP == NULL) {
		_tprintf(TEXT("ERROR: MapViewOfFile (%d)\n"), GetLastError());
		return 1;
	}
	
	while (1)
	{
		//_tprintf(TEXT("\nANTES DO WAIT!!!!"));
		WaitForSingleObject(hEventRead, INFINITE);
		//_tprintf(TEXT("\nPASSEI O WAIT!!!!"));
		WaitForSingleObject(hMutexRead, INFINITE);
		//_tprintf(TEXT("\nPASSEI O MUTEX!!!!"));
		CopyMemory(empresasArry, emP, sizeof(emP) * MAX_EMPRESAS);
		ReleaseMutex(hMutexRead);
		//_tprintf(TEXT("\nPASSEI O WAIT!!!!"));

	}
	return 1;
}