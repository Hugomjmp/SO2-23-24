﻿#include "..\\Trabalho_Prático2024\utils.h"

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
	userData users[6];
	empresaData empresas[30];
	carteiraAcoes cartAcoes[30];
	boardData board = { .empresas = empresas, .cartAcoes = cartAcoes };
	boardData* boardDt;
	//empresaData* emP; 
	ControlData ctrlData = {.empresas = empresas, .users = users, .cartAcoes = cartAcoes };
	//clienteResposta cliRes = {.empresas = empresas };

	//ControlData* cdata;
	
#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif
	//###############################################################
	//#																#
	//#					INICIALIZAR ARRAY CARTEIRA					#
	//#																#
	//###############################################################
	for (int i = 0; i < 30; i++) {
		_tcscpy(cartAcoes[i].nomeEmpresa, TEXT("-1"));
		_tcscpy(cartAcoes[i].username, TEXT("-1"));
		cartAcoes[i].nAções = 0;
		cartAcoes[i].valor = 0;
	}
	//---------------------------------------------------------------
	//###############################################################
	//#																#
	//#					INICIALIZAR ARRAY USER						#
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
			sizeof(boardData),		// (mais significativo e menos significativo)
			SHM_NAME				// Nome a dar ao recurso (fich. mapeado)
	);				
	if (hMapFile == NULL) {
		_tprintf(TEXT("Error: CreateFileMapping (%d)\n"), GetLastError());
		return 1;
	}

	/*emP = (empresaData*)*/boardDt = (boardData*)MapViewOfFile(
			hMapFile,				// Handle do ficheiro mapeado
			FILE_MAP_ALL_ACCESS,	// Flags de acesso (ler, escrever)
			0,						// Início dentro do bloco pretendido
			0,						// dentro do ficheiro (+signific., -signific.)
			sizeof(boardData)/*sizeof(empresaData)*/		// Tamanho da view pretendida
	);
	if (/*emP*/boardDt == NULL) {
		_tprintf(TEXT("ERROR: MapViewOfFile MAIN(%d)\n"), GetLastError());
		CloseHandle(hMapFile);
		return 1;
	}
	for (DWORD i = 0; i < MAX_EMPRESAS; i++)
	{
		_tcscpy(boardDt->empresas[i].nomeEmpresa, TEXT("-1"));
		boardDt->empresas[i].nAções = 0;
		boardDt->empresas[i].pAção = 0.0;
		_tcscpy(boardDt->cartAcoes[i].nomeEmpresa, TEXT("-1"));
		_tcscpy(boardDt->cartAcoes[i].username, TEXT("-1"));
		boardDt->cartAcoes[i].nAções = 0;
		boardDt->cartAcoes[i].valor = 0.0;
		//_tcscpy(emP[i].nomeEmpresa, TEXT("-1"));
		//emP[i].nAções = 0;
		//emP[i].pAção = 0.0;
	}
	_tprintf(TEXT("\nCARTEIRA %d"), sizeof(boardData));


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
		mostraMenu();
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

							_tcscpy(boardDt->empresas[i].nomeEmpresa, nomeEmpresa);
							boardDt->empresas[i].nAções = nAções;
							boardDt->empresas[i].pAção = pAção;
						
						CopyMemory(boardDt->empresas/*emP*/, empresas, sizeof(empresaData)/*sizeof(empresaData) * MAX_EMPRESAS*/);
						
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
				_tcscpy(boardDt->empresas[linha].nomeEmpresa, empresas[linha].nomeEmpresa);
				boardDt->empresas[linha].nAções = empresas[linha].nAções;
				boardDt->empresas[linha].pAção = empresas[linha].pAção;
				linha++;
			}

			//bloqueia no mutex
			WaitForSingleObject(hMutex, INFINITE);
			//copia dados para a sharedmemory
			CopyMemory(boardDt->empresas/*emP*/, empresas, sizeof(empresaData));
			//CopyMemory(emP, empresas, sizeof(empresaData) * MAX_EMPRESAS);
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
						CopyMemory(boardDt->empresas/*boardDt*//*emP*/, empresas, sizeof(empresaData));
						//CopyMemory(emP, empresas, sizeof(empresaData)* MAX_EMPRESAS);
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
	UnmapViewOfFile(/*emP*/boardDt);
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
	DWORD n, i = 0, id, ret, registo = 0, resposta = 0, totalAcoes = 0, resLogin = 0;
	clienteData cliData;
	DWORD NCLIENTES = leRegedit();
	TCHAR nomeEmpresa[50];
	DWORD nAções;
	empresaData lastTransation;
	carteiraAcoes ctA[30];
	//empresaData empresas[30];
	clienteResposta cliRes;
	carteiraAcoes* cA;
	//novo
	HANDLE hMapFile;
	boardData* boardDt;

	DWORD index = 0;
	float  totalPacoes = 0, valorAtual = 0, valorTotal = 0, venda = 0, vendaAcao = 0, lucro = 0;
	WaitForSingleObject(ptd_extra->ptd->hTrinco, INFINITE);
	id = ptd_extra->id;
	hPipesloc = ptd_extra->ptd->hPipe[id];
	ReleaseMutex(ptd_extra->ptd->hTrinco);
	for (int i = 0; i < 30; i++) {
		_tcscpy(ctA[i].nomeEmpresa, TEXT("-1"));
		_tcscpy(ctA[i].username, TEXT("-1"));
		ctA[i].nAções = 0;
		ctA[i].valor = 0;
	}
	//###############################################################
	//#																#
	//#						Shared Memory	 						#
	//#																#
	//###############################################################
	hMapFile = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,	// acesso pretendido 
		FALSE,
		SHM_NAME			// nome dado ao recurso (ficheiro mapeado)
	);
	if (hMapFile == NULL) {
		_tprintf(TEXT("Error: OpenFileMapping (%d)\n"), GetLastError());
		return 1;
	}
	/*emP = (empresaData*)*/boardDt = (boardData*)MapViewOfFile(
		hMapFile,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		sizeof(boardData)
	);
	//###############################################################
	//#																#
	//#							MUTEX								#
	//#																#
	//###############################################################
	HANDLE hMutex = OpenMutex(
		MUTEX_ALL_ACCESS,
		FALSE,
		MUTEX_NAME);

	if (hMutex == NULL) {
		_tprintf(TEXT("ERROR: Mutex (%d)\n"), GetLastError());
		return 1;
	}

	//###############################################################
	//#																#
	//#						Eventos			 						#
	//#																#
	//###############################################################
	HANDLE hEvent = OpenEvent(
		EVENT_MODIFY_STATE,	//dwDesiredAccess,
		FALSE,			//bInheritHandle,
		EVENT_NAME	//lpName
	);
	if (hEvent == NULL) {
		_tprintf(TEXT("Erro ao abrir o evento. Código de erro: %d\n", GetLastError()));
		return 1;
	}
	//_tprintf(TEXT("\nEscrever no pipe! %s\n"), ptd_extra->users[0].username);
	do {
		index = 0;
		//RECEBE PEDIDO...
		
		ret = ReadFile(hPipesloc, &cliData, sizeof(clienteData), &n, NULL);
		if (!ret || !n) {
			_tprintf(TEXT("[ERRO] %d %d... (ReadFile)\n"), ret, n);
			break;
		}

		//_tprintf(TEXT("[SERVIDOR] Recebi %d bytes: '%s' do cliente %d... (ReadFile)\n"), n, buf, id);
		//_tprintf(TEXT("[SERVIDOR] Recebi %d bytes: '%s' '%s' do cliente %d... (ReadFile)\n"), n, cliData.login, cliData.password, id);
		//PROCESSA PEDIDO
		//_tcscpy(buf,ptd_extra->users[0].username);
		
		//ENVIA RESPOSTA...
		/*		if (!WriteFile(hPipesloc, buf, (DWORD)_tcslen(buf) * sizeof(TCHAR), &n, NULL)) {
			_tprintf(TEXT("[ERRO] Escrever no pipe! (WriteFile)\n"));
			exit(-1);
		}*/
		_tcscpy(cliRes.RESPOSTA, TEXT("-1"));

		for (DWORD i = 0; i < MAX_CLIENTES; i++)
		{
			if (_tcsicmp(ptd_extra->users[i].username, cliData.login) == 0 
				&& _tcsicmp(ptd_extra->users[i].password, cliData.password) == 0)
			{
				ptd_extra->users[i].estado = 1;
				index = 1;
				//_tcscpy(cliData.RESPOSTA, ptd_extra->users[i].username);
				_tcscpy(cliRes.RESPOSTA, TEXT("1"));

				if (!WriteFile(hPipesloc, &cliRes, sizeof(clienteResposta), &n, NULL)) {
					_tprintf(TEXT("[ERRO] Escrever no pipe! (WriteFile)\n"));
					exit(-1);
				}
				
			}
			
		}
		if (_tcsicmp(cliRes.RESPOSTA, TEXT("-1")) == 0) {
			if (!WriteFile(hPipesloc, &cliRes, sizeof(clienteResposta), &n, NULL)) {
				_tprintf(TEXT("[ERRO] Escrever no pipe! (WriteFile)\n"));
				exit(-1);
			}
		}


		_tprintf(TEXT("\n[ERRO] ESTADO: %d\n"), index);
		//_tprintf(TEXT("\n[SERVIDOR] Enviei '%s' %d bytes ao cliente antes do login %d... (WriteFile)\n"), cliData.RESPOSTA, n, id);
	} while (/*_tcscmp(buf, TEXT("SAIR")) != 0*/index!=1/*ptd_extra->users[index].estado != 1*/);
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
		//_tprintf(TEXT("[SERVIDOR] Recebi %d bytes: '%s' do cliente %d... (ReadFile)\n"), n, cliData.comando, id);
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
			for (DWORD i = 0; i < MAX_EMPRESAS; i++)
			{
				_tcscpy(cliRes.empW[i].nomeEmpresa, ptd_extra->empresas[i].nomeEmpresa);
				cliRes.empW[i].nAções = ptd_extra->empresas[i].nAções;
				cliRes.empW[i].pAção = ptd_extra->empresas[i].pAção;
			}
			
			//_tprintf(TEXT("EMPRESA (%s)\n"), cliRes.empresas[0].nomeEmpresa);
			_tcscpy(cliRes.RESPOSTA, TEXT("2"));

			//cliRes.empresas = ptd_extra->empresas;
			//clienteData cliD = { .emp = ptd_extra->empresas };
			if (!WriteFile(hPipesloc, &cliRes, sizeof(clienteResposta), &n, NULL)) {
				_tprintf(TEXT("[ERRO] Escrever no pipe! (WriteFile)\n"));
				exit(-1);
			}
			_tprintf(TEXT("EMPRESA (%s)\n"), cliRes.empW[0].nomeEmpresa);
		}
		//TRATA DO COMANDO COMPRAR AÇÕES 
		//-------------------ACABAR-------------------------//
		else if (_tcsncmp(TEXT("buy"), cliData.comando, _tcslen(TEXT("buy"))) == 0) {
			registo = 0;
			resposta = 0;
			//_tprintf(TEXT("\nCOMANDO: %s\n"), cliData.comando);
			_stscanf(cliData.comando, TEXT("buy %s %lu"), nomeEmpresa, &nAções);
			//_tprintf(TEXT("\nnomeEmpresa: %s\n"), nomeEmpresa);
			//_tprintf(TEXT("nAções: %lu\n"), nAções);
			for (DWORD i = 0; i < MAX_EMPRESAS; i++) {
				
				if ((_tcsicmp(ptd_extra->empresas[i].nomeEmpresa, nomeEmpresa)) == 0
					&& ptd_extra->empresas[i].nAções > 0) {
					for (DWORD a = 0; a < 6; a++)//localizar o utilizador para descontar no saldo
					{
						//_tprintf(TEXT("user: %s\n"), ptd_extra->users[a].username);
						if ((_tcsicmp(ptd_extra->users[a].username, cliData.login) == 0)
							&& ptd_extra->users[a].saldo >= (ptd_extra->empresas[i].pAção * nAções)
							&& (_tcsicmp(ptd_extra->users[a].username, cliData.login)) == 0) {
							ptd_extra->users[a].saldo = ptd_extra->users[a].saldo - ptd_extra->empresas[i].pAção * nAções;
							for (DWORD j = 0; j < 30; j++)
							{
								
								if (registo == 0 && (_tcsicmp(ptd_extra->cartAcoes[j].nomeEmpresa, TEXT("-1"))) == 0)
									 {
									//bloqueia no mutex
									WaitForSingleObject(hMutex, INFINITE);
									_tcscpy(ptd_extra->cartAcoes[j].nomeEmpresa, nomeEmpresa);
									_tcscpy(ptd_extra->cartAcoes[j].username, cliData.login);
									ptd_extra->cartAcoes[j].nAções += nAções;
									ptd_extra->cartAcoes[j].valor += ptd_extra->empresas[i].pAção * nAções;
									ptd_extra->empresas[i].pAção *= ((0.01 * nAções) + 1); //aumenta 1% e tem em consideração o Nações compradas
									//_tprintf(TEXT("PREÇO ACOES!!: %f\n"), ptd_extra->empresas[i].pAção);
									registo = 1;
									resposta = 1;
									ptd_extra->empresas[i].nAções -= nAções; // falta mutex aqui nesta instrução
									CopyMemory(boardDt->cartAcoes, ptd_extra->cartAcoes, sizeof(carteiraAcoes));
									//relase ao mutex
									ReleaseMutex(hMutex);
									//evento
									SetEvent(hEvent);
									Sleep(500);
									//resetEvento
									ResetEvent(hEvent);
								}
								else if (registo == 0 && (_tcsicmp(ptd_extra->cartAcoes[j].nomeEmpresa, nomeEmpresa)) == 0
									&& (_tcsicmp(ptd_extra->cartAcoes[j].username, cliData.login)) == 0) {
									//bloqueia no mutex
									WaitForSingleObject(hMutex, INFINITE);
									_tcscpy(ptd_extra->cartAcoes[j].nomeEmpresa, nomeEmpresa);
									_tcscpy(ptd_extra->cartAcoes[j].username, cliData.login);
									ptd_extra->cartAcoes[j].nAções += nAções;
									ptd_extra->cartAcoes[j].valor += ptd_extra->empresas[i].pAção * nAções;
									ptd_extra->empresas[i].pAção *= ((0.01 * nAções) + 1); //aumenta 1% e tem em consideração o Nações compradas
									//_tprintf(TEXT("PREÇO ACOES!!: %f\n"), ptd_extra->empresas[i].pAção);
									registo = 1;
									resposta = 1;
									ptd_extra->empresas[i].nAções -= nAções; // falta mutex aqui nesta instrução
									CopyMemory(boardDt->cartAcoes, ptd_extra->cartAcoes, sizeof(carteiraAcoes));
									//relase ao mutex
									ReleaseMutex(hMutex);
									//evento
									SetEvent(hEvent);
									Sleep(500);
									//resetEvento
									ResetEvent(hEvent);
								}
								else {}
							}

						
						}
					}

				}
			}


			_tprintf(TEXT("\ntabela\n"));
			for (DWORD y = 0; y < 30; y++)
			{
						
				_tprintf(TEXT("|%s\t"), ptd_extra->cartAcoes[y].nomeEmpresa);
				_tprintf(TEXT("%s\t"), ptd_extra->cartAcoes[y].username);
				_tprintf(TEXT("%lu\t"), ptd_extra->cartAcoes[y].nAções);
				_tprintf(TEXT("%.4f|\n"), ptd_extra->cartAcoes[y].valor);
			}

			if (resposta == 1)
			{
				_tcscpy(cliRes.RESPOSTA, TEXT("OK!\n"));
				if (!WriteFile(hPipesloc, &cliRes, sizeof(clienteResposta), &n, NULL)) {
					_tprintf(TEXT("[ERRO] Escrever no pipe! (WriteFile)\n"));
					exit(-1);
				}
			}
			if (resposta == 0)
			{
				_tcscpy(cliRes.RESPOSTA, TEXT("\nSem saldo para realizar a compra!\n"));
				if (!WriteFile(hPipesloc, &cliRes, sizeof(clienteResposta), &n, NULL)) {
					_tprintf(TEXT("[ERRO] Escrever no pipe! (WriteFile)\n"));
					exit(-1);
				}
			}
		}
		//TRATA DO COMANDO VENDER AÇÕES
		//-------------------ACABAR-------------------------//
		else if (_tcsncmp(TEXT("sell"), cliData.comando, _tcslen(TEXT("sell"))) == 0) {
			registo = 0;
			resposta = 0;
			totalAcoes = 0;
			
			_stscanf(cliData.comando, TEXT("sell %s %lu"), nomeEmpresa, &nAções);
			//_tprintf(TEXT("\nnomeEmpresa: %s\n"), nomeEmpresa);
			//_tprintf(TEXT("nAções: %lu\n"), nAções); //quantidade a vender ex 100
			for (DWORD i = 0; i < MAX_EMPRESAS; i++) {

				if ((_tcsicmp(ptd_extra->empresas[i].nomeEmpresa, nomeEmpresa)) == 0) {
					//_tprintf(TEXT("EMPRESA: %s\n"), ptd_extra->empresas[i].nomeEmpresa);
					for (DWORD a = 0; a < 6; a++)//localizar o utilizador para adicionar ao saldo
					{
						if ((_tcsicmp(ptd_extra->users[a].username, cliData.login)) == 0) {
							for (DWORD j = 0; j < 30; j++)
							{
								if ((_tcsicmp(ptd_extra->cartAcoes[j].nomeEmpresa, nomeEmpresa)) == 0
									&& (_tcsicmp(ptd_extra->cartAcoes[j].username, cliData.login)) == 0) {
									//bloqueia no mutex
									WaitForSingleObject(hMutex, INFINITE);
									ptd_extra->cartAcoes[j].nAções -= nAções;
									valorAtual = ptd_extra->empresas[i].pAção;
									valorTotal = valorAtual * nAções;
									lucro = valorTotal - ptd_extra->cartAcoes[j].valor;
									_tprintf(TEXT("EMPRESA: %s\n"), ptd_extra->empresas[i].nomeEmpresa);
									_tprintf(TEXT("ptd_extra->cartAcoes[j].nAções: %lu\n"), ptd_extra->cartAcoes[j].nAções);
									if (ptd_extra->cartAcoes[j].nAções == 0)
									{
										_tcscpy(ptd_extra->cartAcoes[j].nomeEmpresa, TEXT("-1"));
										_tcscpy(ptd_extra->cartAcoes[j].username, TEXT("-1"));
										ptd_extra->cartAcoes[j].nAções = 0;
										ptd_extra->cartAcoes[j].valor = 0;
										
										for (int i = 0; i < j; ++i) {
											ctA[i] = ptd_extra->cartAcoes[i];
										}

										// Copiar os elementos depois da posição a ser removida
										for (int i = j + 1; i < 30; ++i) {
											ctA[i - 1] = ptd_extra->cartAcoes[i];
										}
										for (DWORD l = 0; l < 30; l++)
										{
											ptd_extra->cartAcoes[l] = ctA[l];
										}
									}



									ptd_extra->empresas[i].pAção /= ((0.01 * nAções) + 1);//retira 1% e tem em consideração o Nações vendidas
									ptd_extra->users[a].saldo += lucro;
									registo = 1;
									resposta = 1;
									ptd_extra->empresas[i].nAções += nAções;
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
					
				}




			}

		
			_tprintf(TEXT("tabela\n"), nAções);
			for (DWORD y = 0; y < 30; y++)
			{

				_tprintf(TEXT("|%s\t"), ptd_extra->cartAcoes[y].nomeEmpresa);
				_tprintf(TEXT("%s\t"), ptd_extra->cartAcoes[y].username);
				_tprintf(TEXT("%lu\t"), ptd_extra->cartAcoes[y].nAções);
				_tprintf(TEXT("%.4f|\n"), ptd_extra->cartAcoes[y].valor);
			}

			if (resposta == 1)
			{
				_tcscpy(cliRes.RESPOSTA, TEXT("OK!\n"));
				if (!WriteFile(hPipesloc, &cliRes, sizeof(clienteResposta), &n, NULL)) {
					_tprintf(TEXT("[ERRO] Escrever no pipe! (WriteFile)\n"));
					exit(-1);
				}
			}
			if (resposta == 0)
			{
				_tcscpy(cliRes.RESPOSTA, TEXT("\nSem saldo para realizar a compra!\n"));
				if (!WriteFile(hPipesloc, &cliRes, sizeof(clienteResposta), &n, NULL)) {
					_tprintf(TEXT("[ERRO] Escrever no pipe! (WriteFile)\n"));
					exit(-1);
				}
			}
		}
		//TRATA DO COMANDO BALANCE
		else if (_tcsicmp(TEXT("balance"), cliData.comando) == 0) {
			for (DWORD i = 0; i < 6; i++)
			{
				if (_tcsicmp(ptd_extra->users[i].username, cliData.login) == 0)
				{
					
					_stprintf(cliRes.RESPOSTA, TEXT("O seu saldo é de %.2f€\n"), ptd_extra->users[i].saldo);
					if (!WriteFile(hPipesloc, &cliRes, sizeof(clienteResposta), &n, NULL)) {
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
		ptd_extra->users[i].estado = 0; //mudar isto depois
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
			td_extra[nCli].cartAcoes = cdata->cartAcoes;
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


//############################################################### //corrigir SHM
//#																#
//#				Variação aleatoria de preços					#
//#																#
//###############################################################
//falta mandar para a SHARED MEMORY!!
DWORD WINAPI variaPreços(LPVOID empresas) {
	//empresaData(*data)[MAX_COLUNA] = (empresaData(*)[MAX_COLUNA])empresas;
	empresaData *empresasArry = (empresaData*)empresas;
	boardData* boardDt;
	//empresaData* emP;
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
	/*emP = (empresaData*)*/boardDt = (boardData*)MapViewOfFile(
		hMapFile,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		/*sizeof(empresaData)*/sizeof(empresaData)
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
				CopyMemory(boardDt->empresas, empresasArry, sizeof(empresaData)/*sizeof(empresaData) * MAX_EMPRESAS*/);
				
				//CopyMemory(emP, empresasArry, sizeof(empresaData) * MAX_EMPRESAS);
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
				CopyMemory(boardDt->empresas, empresasArry, sizeof(empresaData)/*sizeof(empresaData) * MAX_EMPRESAS*/);
				//CopyMemory(emP, empresasArry, sizeof(empresaData) * MAX_EMPRESAS);
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
	boardData* boardDt;
	//empresaData* emP;
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
	/*emP = (empresaData*)*/boardDt = (boardData*)MapViewOfFile(
		hMapFile,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		/*sizeof(empresaData)*/sizeof(empresaData)
	);
	if (boardDt/*emP*/ == NULL) {
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
			//empresasArry[i] = emP[i];
			//empresasArry[i] = boardDt->empresas[i];  // VER O QUE SE PASSA AQUIIIIIIIIIIIII
		}
		//CopyMemory(empresasArry, emP, sizeof(emP) * MAX_EMPRESAS);
		ReleaseMutex(hMutexRead);
		//_tprintf(TEXT("\nPASSEI O WAIT!!!!"));
		Sleep(1000);
	}
	return 1;
}


void mostraMenu() {
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
}