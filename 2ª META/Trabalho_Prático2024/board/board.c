#include "..\\utils.h"

int _tmain(int argc, TCHAR* argv[])
{
	HANDLE hThread;
	empresaData empresasBoard[MAX_EMPRESAS];
	TCHAR comandoBoard[200];

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif
	for (DWORD i = 0; i < MAX_EMPRESAS; i++)
	{
		_tcscpy(empresasBoard[i].nomeEmpresa, TEXT("-1"));
		empresasBoard[i].nAções = 0;
		empresasBoard[i].pAção = 0.0;
	}

	//---------------------------------------------------------------

	hThread = NULL; // é bom inicializar a zero para depois podermos testar se a thread foi criada com sucesso
	hThread = CreateThread(
		NULL,					// default security attributes
		0,						// use default stack size
		Organiza_dados,			// thread function name
		&empresasBoard,			// argument to thread function
		0,						// use default creation flags
		NULL);
	//---------------------------------------------------------------
		while ((_tcsicmp(TEXT("close"), comandoBoard)) != 0)
	{
			_tprintf(TEXT("\n\t\t\t close - Fechar o programa\n"));
			fflush(stdin);
			//_tprintf(TEXT("\n Comando: "));
			_tscanf(TEXT("%s"), comandoBoard);
			if ((_tcsicmp(TEXT("close"), comandoBoard)) != 0)
			{
				break;
			}
			else
			{
				_tprintf(TEXT("\nComando: %s introduzido com tamanho %zu, não existe!"), comandoBoard, _tcslen(comandoBoard));
}
		//-------------------
		
	}
		CloseHandle(hThread);
	return 0;
}

DWORD WINAPI Organiza_dados(LPVOID empresas) {
	HANDLE hEvent, hEventWrite, hMutex, hMutexWrite, hMapFile;
	empresaData* empresasBoard = (empresaData*)empresas;
	//empresaData* emP;
	carteiraAcoes cartA[30];
	boardData* boardDt;
	for (DWORD i = 0; i < MAX_EMPRESAS; i++)
	{
		_tcscpy(cartA[i].nomeEmpresa, TEXT("-1"));
		_tcscpy(cartA[i].username, TEXT("-1"));
		cartA[i].nAções = 0;
		cartA[i].valor = 0.0;
	}

	//###############################################################
	//#																#
	//#						Eventos			 						#
	//#																#
	//###############################################################
	hEvent = OpenEvent(
		SYNCHRONIZE,	//dwDesiredAccess,
		FALSE,			//bInheritHandle,
		EVENT_NAME		//lpName
	);
	if (hEvent == NULL) {
		_tprintf(TEXT("Erro ao abrir o evento. Código de erro: %d\n", GetLastError()));
		return 1;
	}
	
	hEventWrite = OpenEvent(
		EVENT_MODIFY_STATE,	//dwDesiredAccess,
		FALSE,				//bInheritHandle,
		EVENT_NAME_O		//lpName
	);
	if (hEventWrite == NULL) {
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
	hMutexWrite = OpenMutex(
		MUTEX_ALL_ACCESS,
		FALSE,
		MUTEX_NAME_O);

	if (hMutexWrite == NULL) {
		_tprintf(TEXT("ERROR: MutexWrite (%d)\n"), GetLastError());
		return 1;
	}
	//---------------------------------------------------------------
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

	if (boardDt/*emP*/ == NULL) {
		_tprintf(TEXT("ERROR: MapViewOfFile (%d)\n"), GetLastError());
		return 1;
	}
	/*for (DWORD i = 0; i < MAX_EMPRESAS; i++)
	{
		_tprintf(TEXT("\nCARTEIRA DE ACOES DONO: %s"), boardDt->cartAcoes[i].nomeEmpresa);
		_tprintf(TEXT("\nnCARTEIRA DE ACOES user: %s"), boardDt->cartAcoes[i].username);
		_tprintf(TEXT("\nCARTEIRA DE ACOES nacaoes: %d"), boardDt->cartAcoes[i].nAções);
		_tprintf(TEXT("\nCARTEIRA DE ACOES valor: %.2f"), boardDt->cartAcoes[i].valor);
		//_tcscpy(emP[i].nomeEmpresa, TEXT("-1"));
		//emP[i].nAções = 0;
		//emP[i].pAção = 0.0;
	}*/
	while(1){
		WaitForSingleObject(hEvent, INFINITE);
		WaitForSingleObject(hMutex, INFINITE);
		for (DWORD i = 0; i < MAX_EMPRESAS; i++) //copia o que tem na shared memory para um array local
		{
			//empresasBoard[i] = emP[i];
			empresasBoard[i] = boardDt->empresas[i];
			
		}
		//for (DWORD i = 0; i < 30; i++)
		//{
		//	_tprintf(TEXT("\nCARTEIRA DE ACOES DONO: %s"), boardDt->cartAcoes[i].nomeEmpresa);
		//	_tprintf(TEXT("\nnCARTEIRA DE ACOES user: %s"), boardDt->cartAcoes[i].username);
		//	_tprintf(TEXT("\nCARTEIRA DE ACOES nacaoes: %d"), boardDt->cartAcoes[i].nAções);
		//	_tprintf(TEXT("\nCARTEIRA DE ACOES valor: %.2f"), boardDt->cartAcoes[i].valor);
		//	//_tcscpy(emP[i].nomeEmpresa, TEXT("-1"));
		//	//emP[i].nAções = 0;
		//	//emP[i].pAção = 0.0;
		//}
		//REORGANIZAÇÃO.... está a duplicar... problema aqui
		
		//bolha
		for (int i = 0; i < MAX_EMPRESAS - 1; i++) {
			for (int j = 0; j < MAX_EMPRESAS - i - 1; j++) {
				if (empresasBoard[j].pAção < empresasBoard[j + 1].pAção) {
					empresaData temp = empresasBoard[j];
					empresasBoard[j] = empresasBoard[j + 1];
					empresasBoard[j + 1] = temp;
				}
			}
		}

		/*
		for (int i = 0; i < MAX_EMPRESAS - 1; i++) {
			for (int j = 0; j < MAX_EMPRESAS - i - 1; j++) {
				if (empresasBoard[j].pAção < empresasBoard[j + 1].pAção) {
					empresaData temp = empresasBoard[j];
					empresasBoard[j] = empresasBoard[j + 1];
					empresasBoard[j + 1] = temp;
				}
			}
		}*/
		/*
		for (DWORD i = 0; i < MAX_EMPRESAS - 1; i++) {
			DWORD maxIndex = i;
			for (DWORD j = i + 1; j < MAX_EMPRESAS; j++) {
				if (empresasBoard[j].pAção > empresasBoard[maxIndex].pAção) {
					maxIndex = j;
				}
			}
			if (maxIndex != i) {
				empresaData temp = empresasBoard[i];
				empresasBoard[i] = empresasBoard[maxIndex];
				empresasBoard[maxIndex] = temp;
			}
		}*/



		ReleaseMutex(hMutex);
		mostra_tabela(empresasBoard, boardDt);
		WaitForSingleObject(hMutexWrite, INFINITE);
		CopyMemory(boardDt->empresas, empresasBoard, sizeof(empresaData) * MAX_EMPRESAS);
		ReleaseMutex(hMutexWrite);
		SetEvent(hEventWrite);
		//Sleep(500);
		ResetEvent(hEventWrite);
		Sleep(1000);
		

	}
	UnmapViewOfFile(boardDt);
	CloseHandle(hMapFile);
}






void mostra_tabela(empresaData* empresasBoard, boardData *boardDt){
	DWORD Nempresas = 10;
	DWORD contador = 0, numeros;
	DWORD digitos = 0;

	//empresaData empresasCompara[MAX_EMPRESAS];
	//boardData* boardDT = (boardData*)boardDt;

	
	

	_tprintf(TEXT("\n\t\t| ID | |\t NOME\t\t| |\t Num_Ações\t| |\t Preço-Ação\t|\n"));
	_tprintf(TEXT("\t\t---------------------------------------------------------------------------------\n"));
	for (DWORD i = 0; i < Nempresas; i++)
	{
		if (i < 9) { //corrige o espaçamento do ID
			_tprintf(TEXT("\t\t| %d  |"), i + 1);
		}
		else {
			_tprintf(TEXT("\t\t| %d |"), i + 1);
		}
		if (_tcslen(empresasBoard[i].nomeEmpresa) <= 5)
		{
			if (_tcscmp(empresasBoard[i].nomeEmpresa, TEXT("-1")) == 0) {
				_tprintf(TEXT(" |\t   \t\t|"), empresasBoard[i].nomeEmpresa); // coloca esppaços vazios onde está "-1"
			}
			else
				_tprintf(TEXT(" |\t %s \t\t|"), empresasBoard[i].nomeEmpresa);
		}
		else {
			_tprintf(TEXT(" |\t %s \t|"), empresasBoard[i].nomeEmpresa);

		}

		_tprintf(TEXT(" |\t %lu \t\t|"), empresasBoard[i].nAções);
		//contar os digitos para retificar espaçamento do Preço Ação
		digitos = 0;
		numeros = empresasBoard[i].pAção;
		while (numeros >= 1) {
			numeros /= 10;
			digitos++;
		}
		if (digitos >= 2)
			_tprintf(TEXT(" |\t %.2f€ \t|"), empresasBoard[i].pAção);
		else
			_tprintf(TEXT(" |\t %.2f€ \t\t|"), empresasBoard[i].pAção);
		_tprintf(TEXT("\n\t\t---------------------------------------------------------------------------------\n"));
		
	}


	//MOSTRAR A ULTIMA TRANSAÇÃO//
	_tprintf(TEXT("\n\t\t--------------------------------ULTIMA TRANSAÇÃO---------------------------------\n"));
	_tprintf(TEXT("\n\t\t\t|\t NOME\t\t| |\t Num_Ações\t| |\t Preço-Ação\t|\n"));
	_tprintf(TEXT("\t\t---------------------------------------------------------------------------------\n"));
	
	/*for (int i = 0; i < 30; i++)
	{
		//if (_tcsicmp(boardDt->cartAcoes[i].nomeEmpresa,TEXT("-1")) != 0) {
			_tprintf(TEXT("\t\t\t|\t %s\t\t| |\t %lu\t\t| |\t %.2f €\t\t|\n"), boardDt->cartAcoes[i].nomeEmpresa,
				boardDt->cartAcoes[i].nAções, boardDt->cartAcoes[i].valor);
			//break;
		//}
		
	}*/

	_tprintf(TEXT("\n\t\t\t close - Fechar o programa\n"));
}
