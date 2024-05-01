#include "..\\Trabalho_Prático2024\utils.h"

int _tmain(int argc, TCHAR* argv[])
{
	//HANDLES...

	HANDLE hPipe, hThreadArray[5] = {NULL,NULL,NULL,NULL,NULL},
		   hEvent, hMapFile, hMutex, hMutexRead, hEventRead, hSemClientes, hSemBolsa, hMutexClient;

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
	userData users[6];
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
	for (int i = 0; i < 6; i++) {
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
	while (linha < 6 && fwscanf(fpU, TEXT("%s %s %f"), users[linha].username, users[linha].password, &users[linha].saldo) == 3) {
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
	//verifica se o bolsa já está a funcionar
	hSemBolsa = CreateSemaphore(NULL, 1, 1, SEM_BOLSA);
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		_tprintf(TEXT("[ERRO] O Bolsa já se encontra a executar\n"));
		system("pause");
		CloseHandle(hSemBolsa);
		return -1;
	}
	
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
	hMutexClient = CreateMutex(
		NULL,
		FALSE,
		MUTEX_NAME_C
	);

	if (hMutexClient== NULL) {
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
		if (_tcsicmp(TEXT("addc"), comandoAdmin) == 0)
			
		{
			_tscanf(TEXT("%s"), &nomeEmpresa);
			_tscanf(TEXT("%lu"), &nAções);
			_tscanf(TEXT("%f"), &pAção);
			DWORD empresaAdiciona = 1;
			for (DWORD i = 0; i < MAX_EMPRESAS; i++) {
				if (empresaAdiciona == 1) {
					if ((_tcsicmp(TEXT("-1"), empresas[i].nomeEmpresa)) == 0) {

						//bloqueia no mutex
						WaitForSingleObject(hMutex, INFINITE);
						_tcscpy(empresas[i].nomeEmpresa, nomeEmpresa);
						empresas[i].nAções = nAções;
						empresas[i].pAção = pAção;
						empresaAdiciona = 0;
						//copia dados para a sharedmemory
						CopyMemory(emP, empresas, sizeof(empresaData)* MAX_EMPRESAS);
						//relase ao mutex
						ReleaseMutex(hMutex);
						
						//evento
						SetEvent(hEvent);
						Sleep(500);
						//resetEvento
						ResetEvent(hEvent);

					}
				}
			}
		}
		//TRATA DO COMANNDO ACRESCENTAR UM EMPRESA via file txt
		if (_tcsicmp(TEXT("adde"), comandoAdmin) == 0)
		{
			//o fopen nao converte logo para utf8 tem de se colocar os parametros
			fp = fopen(EMPRESAS, "rt+, ccs=UTF-8"); //abertura do ficheiro Clientes.txt para leitura
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
			CopyMemory(emP, empresas, sizeof(empresaData) * MAX_EMPRESAS);
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
						CopyMemory(emP, empresas, sizeof(empresaData)* MAX_EMPRESAS);
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
		else if ((_tcsicmp(TEXT("close"), comandoAdmin) == 0)) {
		}
		//TRATA DA FALHA DO COMANDO
		else {
			
			//_tprintf(TEXT("\nComando: %s introduzido com tamanho %zu, não existe!"), comandoAdmin, _tcslen(comandoAdmin));
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
DWORD WINAPI trataComandosClientes(LPVOID data) {
	tDataInfo_EXTRA* ptd_extra = (tDataInfo_EXTRA*)data;
	HANDLE hPipesloc;
	TCHAR buf[256];
	DWORD n, i = 0, id, ret;
	clienteData cliData;
	DWORD NCLIENTES = leRegedit();
	TCHAR nomeEmpresa[50];
	float nAções;
	WaitForSingleObject(ptd_extra->ptd->hTrinco, INFINITE);
	id = ptd_extra->id;
	hPipesloc = ptd_extra->ptd->hPipe[id];
	ReleaseMutex(ptd_extra->ptd->hTrinco);
	//_tprintf(TEXT("\nEscrever no pipe! %s\n"), ptd_extra->users[0].username);
	do {
		//RECEBE PEDIDO...
		//ret = ReadFile(hPipesloc, buf, sizeof(buf), &n, NULL);
		ret = ReadFile(hPipesloc, &cliData, sizeof(clienteData), &n, NULL);
		//buf[n / sizeof(TCHAR)] = _T('\0');
		if (!ret || !n) {
			_tprintf(TEXT("[ERRO] %d %d... (ReadFile)\n"), ret, n);
			break;
		}

		//_tprintf(TEXT("[SERVIDOR] Recebi %d bytes: '%s' do cliente %d... (ReadFile)\n"), n, buf, id);
		_tprintf(TEXT("[SERVIDOR] Recebi %d bytes: '%s' '%s' do cliente %d... (ReadFile)\n"), n, cliData.login, cliData.password, id);
		//PROCESSA PEDIDO
		//CharUpperBuff(buf, (DWORD)_tcslen(buf));
		//_tcscpy(buf,ptd_extra->users[0].username);
		
		//ENVIA RESPOSTA...
		/*		if (!WriteFile(hPipesloc, buf, (DWORD)_tcslen(buf) * sizeof(TCHAR), &n, NULL)) {
			_tprintf(TEXT("[ERRO] Escrever no pipe! (WriteFile)\n"));
			exit(-1);
		}*/
		for (DWORD i = 0; i < MAX_CLIENTES; i++)
		{
			if (_tcsicmp(ptd_extra->users[i].username, cliData.login) == 0 && _tcsicmp(ptd_extra->users[i].password, cliData.password) == 0)
			{
				ptd_extra->users[i].estado = 1;
				//_tcscpy(cliData.RESPOSTA, ptd_extra->users[i].username);
				_tcscpy(cliData.RESPOSTA, TEXT("1"));
				if (!WriteFile(hPipesloc, &cliData, sizeof(clienteData), &n, NULL)) {
					_tprintf(TEXT("[ERRO] Escrever no pipe! (WriteFile)\n"));
					exit(-1);
				}
			}
		}


		_tprintf(TEXT("[SERVIDOR] Enviei %d bytes ao cliente %d... (WriteFile)\n"), n, id);
	} while (/*_tcscmp(buf, TEXT("SAIR")) != 0*/ptd_extra->users[i].estado != 1);
	do {
		//RECEBE PEDIDO...
		//ret = ReadFile(hPipesloc, buf, sizeof(buf), &n, NULL);
		ret = ReadFile(hPipesloc, &cliData, sizeof(clienteData), &n, NULL);
		//buf[n / sizeof(TCHAR)] = _T('\0');
		if (!ret || !n) {
			_tprintf(TEXT("[ERRO] %d %d... (ReadFile)\n"), ret, n);
			break;
		}

		//_tprintf(TEXT("[SERVIDOR] Recebi %d bytes: '%s' do cliente %d... (ReadFile)\n"), n, buf, id);
		_tprintf(TEXT("[SERVIDOR] Recebi %d bytes: '%s' do cliente %d... (ReadFile)\n"), n, cliData.comando, id);
		//PROCESSA PEDIDO
		//CharUpperBuff(buf, (DWORD)_tcslen(buf));
		//_tcscpy(buf,ptd_extra->users[0].username);

		//ENVIA RESPOSTA...
		/*		if (!WriteFile(hPipesloc, buf, (DWORD)_tcslen(buf) * sizeof(TCHAR), &n, NULL)) {
			_tprintf(TEXT("[ERRO] Escrever no pipe! (WriteFile)\n"));
			exit(-1);
		}*/
		//TRATA DO COMANDO LISTAR TODAS AS EMPRESAS
		if (_tcsicmp(TEXT("listc"), cliData.comando) == 0) {
			_tcscpy(cliData.RESPOSTA, cliData.comando);
			if (!WriteFile(hPipesloc, &cliData, sizeof(clienteData), &n, NULL)) {
				_tprintf(TEXT("[ERRO] Escrever no pipe! (WriteFile)\n"));
				exit(-1);
			}
		}
		//TRATA DO COMANDO COMPRAR AÇÕES 
		//-------------------ACABAR-------------------------//
		else if (_tcsncmp(TEXT("buy"), cliData.comando, _tcslen(TEXT("buy"))) == 0) {
			_tprintf(TEXT("\nCOMANDO: %s\n"), cliData.comando);
			_stscanf(cliData.comando, TEXT("buy %s %f"), nomeEmpresa, &nAções);
			_tprintf(TEXT("\nnomeEmpresa: %s\n"), nomeEmpresa);
			_tprintf(TEXT("nAções: %.2f\n"), nAções);
			for (DWORD i = 0; i < MAX_EMPRESAS; i++) {
				if ((_tcsicmp(ptd_extra->empresas[i].nomeEmpresa, nomeEmpresa)) == 0) {
					_tprintf(TEXT("EMPRESA: %s\n"), ptd_extra->empresas[i].nomeEmpresa);
					ptd_extra->empresas[i].nAções += nAções;
				}
			}
			
			_tcscpy(cliData.RESPOSTA, TEXT("\nOK!\n"));
			if (!WriteFile(hPipesloc, &cliData, sizeof(clienteData), &n, NULL)) {
				_tprintf(TEXT("[ERRO] Escrever no pipe! (WriteFile)\n"));
				exit(-1);
			}
		}
		//TRATA DO COMANDO VENDER AÇÕES
		//-------------------ACABAR-------------------------//
		else if (_tcsncmp(TEXT("sell"), cliData.comando, _tcslen(TEXT("sell"))) == 0) {
			_tprintf(TEXT("\nCOMANDO: %s\n"), cliData.comando);
			_stscanf(cliData.comando, TEXT("sell %s %f"), nomeEmpresa, &nAções);
			_tprintf(TEXT("\nnomeEmpresa: %s\n"), nomeEmpresa);
			_tprintf(TEXT("nAções: %.2f\n"), nAções);
			for (DWORD i = 0; i < MAX_EMPRESAS; i++) {
				if ((_tcsicmp(ptd_extra->empresas[i].nomeEmpresa, nomeEmpresa)) == 0) {
					_tprintf(TEXT("EMPRESA: %s\n"), ptd_extra->empresas[i].nomeEmpresa);
					ptd_extra->empresas[i].nAções -= nAções;
					ptd_extra->empresas
				}
			}

			_tcscpy(cliData.RESPOSTA, TEXT("\nOK!\n"));
			if (!WriteFile(hPipesloc, &cliData, sizeof(clienteData), &n, NULL)) {
				_tprintf(TEXT("[ERRO] Escrever no pipe! (WriteFile)\n"));
				exit(-1);
			}
		}
		//TRATA DO COMANDO BALANCE
		else if (_tcsicmp(TEXT("balance"), cliData.comando) == 0) {
			for (DWORD i = 0; i < 6; i++)
			{
				if (_tcsicmp(ptd_extra->users[i].username, cliData.login) == 0)
				{
					
					_stprintf(cliData.RESPOSTA, TEXT("O seu saldo é de %.2f€"), ptd_extra->users[i].saldo);
					if (!WriteFile(hPipesloc, &cliData, sizeof(clienteData), &n, NULL)) {
						_tprintf(TEXT("[ERRO] Escrever no pipe! (WriteFile)\n"));
						exit(-1);
					}
				}
			}


		}
		//TRATA DA FALHA DO COMANDO
		else {
			_tprintf(TEXT("\nComando do cliente: %s introduzido com tamanho %d, não existe!"), cliData.comando, _tcslen(cliData.comando) - 1);
		}




		_tprintf(TEXT("[SERVIDOR] Enviei %d bytes ao cliente %d... (WriteFile)\n"), n, id);
	} while (_tcscmp(cliData.comando, TEXT("exit")) != 0);
	_tprintf(TEXT("[SERVIDOR] Desligar o pipe %d (DisconnectNamedPipe)\n"), id);
	FlushFileBuffers(hPipesloc);
	WaitForSingleObject(ptd_extra->ptd->hTrinco, INFINITE);
	ptd_extra->ptd->hPipe[id] = NULL;
	ReleaseMutex(ptd_extra->ptd->hTrinco);
	if (!DisconnectNamedPipe(hPipesloc)) {
		_tprintf(TEXT("[ERRO] Desligar o pipe! (DisconnectNamedPipe)"));
		exit(-1);
	}
	CloseHandle(hPipesloc);
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
	DWORD n;
	HANDLE hPipe;
	int i = 0;
	TCHAR buf[256];
	//HANDLE tEscrita;
	DWORD nCli = 0;
	tDataInfo_EXTRA td_extra[5];
	HANDLE hThreadCli[5];

	tDataInfo tWrite;

	tWrite.hTrinco = CreateMutex(NULL, FALSE, NULL);
	
	//INICIALIZAR ... COLOCAR ARRAY DE HANDLES A NULL
	for (i = 0; i < 5; i++)
		tWrite.hPipe[i] = NULL;


	while (1) {
		//_tprintf(TEXT("[ESCRITOR] Criar copia %d do pipe '%s' ... (CreateNamedPipe)\n"), nCli, NAME_PIPE);
		hPipe = CreateNamedPipe(NAME_PIPE, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_WAIT |
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, PIPE_UNLIMITED_INSTANCES,
			sizeof(buf), sizeof(buf), 1000, NULL);

		if (hPipe == INVALID_HANDLE_VALUE) {
			wprintf(TEXT("[ERRO] Criar Named Pipe! (CreateNamedPipe)"));
			exit(-1);
		}
		/*
		if (GetLastError() == ERROR_PIPE_BUSY) { //PODIA SER ASSIM TAMBÉM
			//TODOS OS CLIENTES CONECTADOS
			_tprintf(TEXT("TODOS OS CLIENTES ESTAO CONECTADOS"));
		}
		*/
		//_tprintf(TEXT("[ESCRITOR] Esperar ligacao de um cliente %d... (ConnectNamedPipe)\n"), nCli);
		if (!ConnectNamedPipe(hPipe, NULL)) {
			_tprintf(TEXT("[ERRO] Ligação ao cliente! (ConnectNamedPipe\n"));
			exit(-1);
		}

		if (nCli < 5) {
			WaitForSingleObject(tWrite.hTrinco, INFINITE);
			tWrite.hPipe[nCli] = hPipe;
			ReleaseMutex(tWrite.hTrinco);
			//cdata[nCli]->ptd = &tWrite;
			//cdata[nCli]->id = nCli;
			td_extra[nCli].ptd = &tWrite;
			td_extra[nCli].id = nCli;
			td_extra[nCli].empresas = cdata->empresas;
			td_extra[nCli].users = cdata->users;
			//LANÇAR UMA THREAD PARA CADA CLIENTE
			hThreadCli[nCli] = CreateThread(NULL, 0, trataComandosClientes, (LPVOID)&td_extra[nCli], 0, NULL);
			//hThreadCli[nCli] = CreateThread(NULL, 0, trataComandosClientes, (LPVOID)&cdata[nCli], 0, NULL);
			nCli++;
		}
		else {
			DisconnectNamedPipe(hPipe);
		}

	}

	/*
	for(i =0; i<MAX; i++){
		if(tWrite.hPipes[i]!=NULL){
		   WaitForSingleObject(hThreadCli[i], INFINITE);
		   CloseHandle(hThreadCli[i]);
		}
	}
	*/

	//WaitForSingleObject(tEscrita, INFINITE);
	//CloseHandle(tEscrita);
	CloseHandle(tWrite.hTrinco);
	return 0;
	return 0;
}


//###############################################################
//#																#
//#				Variação aleatoria de preços					#
//#																#
//###############################################################
//falta mandar para a SHARED MEMORY!!
DWORD WINAPI variaPreços(LPVOID empresas) {
	//empresaData(*data)[MAX_COLUNA] = (empresaData(*)[MAX_COLUNA])empresas;
	empresaData *empresasArry = (empresaData*)empresas;
	empresaData* emP;
	DWORD contador = 0;
	DWORD linhaAleatoria = 0;
	DWORD tipoAlteração = 0;

	HANDLE hMutex, hEvent, hMapFile;


	//###############################################################
	//#																#
	//#						Eventos			 						#
	//#																#
	//###############################################################
	hEvent = OpenEvent(
		EVENT_MODIFY_STATE,	//dwDesiredAccess,
		FALSE,			//bInheritHandle,
		EVENT_NAME	//lpName
	);
	if (hEvent == NULL) {
		_tprintf(TEXT("Erro ao abrir o evento. Código de erro: %d\n", GetLastError()));
		return 1;
	}
	//###############################################################
	//#																#
	//#							MUTEX								#
	//#																#
	//###############################################################
	hMutex = OpenMutex(
		MUTEX_ALL_ACCESS,
		FALSE,
		MUTEX_NAME);

	if (hMutex == NULL) {
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
				//bloqueia no mutex
				WaitForSingleObject(hMutex, INFINITE);
				//copia dados para a sharedmemory
				CopyMemory(emP, empresasArry, sizeof(empresaData) * MAX_EMPRESAS);
				//relase ao mutex
				ReleaseMutex(hMutex);

				//evento
				SetEvent(hEvent);
				Sleep(500);
				//resetEvento
				ResetEvent(hEvent);
			}
			else
			{
				linhaAleatoria = rand() % contador;
				empresasArry[linhaAleatoria].pAção = empresasArry[linhaAleatoria].pAção + 
					(empresasArry[linhaAleatoria].pAção * 0.2); //aumenta 20%
				//bloqueia no mutex
				WaitForSingleObject(hMutex, INFINITE);
				//copia dados para a sharedmemory
				CopyMemory(emP, empresasArry, sizeof(empresaData) * MAX_EMPRESAS);
				//relase ao mutex
				ReleaseMutex(hMutex);

				//evento
				SetEvent(hEvent);
				Sleep(500);
				//resetEvento
				ResetEvent(hEvent);
			}

		}

		contador = 0;
		Sleep(10000); //10segundos alterar se for o caso
	}
	return 30;
}

//---------------------------------------------------------------

//###############################################################
//#																#
//#				Coloca valores atualizados no array				#
//#							de empresas							#
//###############################################################
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
		for (DWORD i = 0; i < MAX_EMPRESAS; i++) //copia o que tem na shared memory para um array local
		{
			empresasArry[i] = emP[i];
		}
		//CopyMemory(empresasArry, emP, sizeof(emP) * MAX_EMPRESAS);
		ReleaseMutex(hMutexRead);
		//_tprintf(TEXT("\nPASSEI O WAIT!!!!"));
		Sleep(1000);
	}
	return 1;
}