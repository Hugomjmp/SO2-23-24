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
		empresasBoard[i].nA��es = 0;
		empresasBoard[i].pA��o = 0.0;
	}

	//---------------------------------------------------------------

	hThread = NULL; // � bom inicializar a zero para depois podermos testar se a thread foi criada com sucesso
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
			if ((_tcsicmp(TEXT("close"), comandoBoard)) == 0)
			{
				break;
			}
			else
			{
				_tprintf(TEXT("\nComando: '%s' introduzido, n�o existe!"), comandoBoard);
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
		cartA[i].nA��es = 0;
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
		_tprintf(TEXT("Erro ao abrir o evento. C�digo de erro: %d\n", GetLastError()));
		return 1;
	}
	
	hEventWrite = OpenEvent(
		EVENT_MODIFY_STATE,	//dwDesiredAccess,
		FALSE,				//bInheritHandle,
		EVENT_NAME_O		//lpName
	);
	if (hEventWrite == NULL) {
		_tprintf(TEXT("Erro ao abrir o evento. C�digo de erro: %d\n", GetLastError()));
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
		_tprintf(TEXT("\nCARTEIRA DE ACOES nacaoes: %d"), boardDt->cartAcoes[i].nA��es);
		_tprintf(TEXT("\nCARTEIRA DE ACOES valor: %.2f"), boardDt->cartAcoes[i].valor);
		//_tcscpy(emP[i].nomeEmpresa, TEXT("-1"));
		//emP[i].nA��es = 0;
		//emP[i].pA��o = 0.0;
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
		//	_tprintf(TEXT("\nCARTEIRA DE ACOES nacaoes: %d"), boardDt->cartAcoes[i].nA��es);
		//	_tprintf(TEXT("\nCARTEIRA DE ACOES valor: %.2f"), boardDt->cartAcoes[i].valor);
		//	//_tcscpy(emP[i].nomeEmpresa, TEXT("-1"));
		//	//emP[i].nA��es = 0;
		//	//emP[i].pA��o = 0.0;
		//}
		//REORGANIZA��O.... est� a duplicar... problema aqui
		
		//bolha
		for (int i = 0; i < MAX_EMPRESAS - 1; i++) {
			for (int j = 0; j < MAX_EMPRESAS - i - 1; j++) {
				if (empresasBoard[j].pA��o < empresasBoard[j + 1].pA��o) {
					empresaData temp = empresasBoard[j];
					empresasBoard[j] = empresasBoard[j + 1];
					empresasBoard[j + 1] = temp;
				}
			}
		}

		/*
		for (int i = 0; i < MAX_EMPRESAS - 1; i++) {
			for (int j = 0; j < MAX_EMPRESAS - i - 1; j++) {
				if (empresasBoard[j].pA��o < empresasBoard[j + 1].pA��o) {
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
				if (empresasBoard[j].pA��o > empresasBoard[maxIndex].pA��o) {
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
	DWORD contador = 0, numeros, numerosT;
	DWORD digitos = 0, digitosT = 0;

	//empresaData empresasCompara[MAX_EMPRESAS];
	//boardData* boardDT = (boardData*)boardDt;

	
	

	_tprintf(TEXT("\n\t\t| ID | |\t NOME\t\t| |\t Num_A��es\t| |\t Pre�o-A��o\t|\n"));
	_tprintf(TEXT("\t\t---------------------------------------------------------------------------------\n"));
	for (DWORD i = 0; i < Nempresas; i++)
	{
		if (i < 9) { //corrige o espa�amento do ID
			_tprintf(TEXT("\t\t| %d  |"), i + 1);
		}
		else {
			_tprintf(TEXT("\t\t| %d |"), i + 1);
		}
		if (_tcslen(empresasBoard[i].nomeEmpresa) <= 5)
		{
			if (_tcscmp(empresasBoard[i].nomeEmpresa, TEXT("-1")) == 0) {
				_tprintf(TEXT(" |\t   \t\t|"), empresasBoard[i].nomeEmpresa); // coloca esppa�os vazios onde est� "-1"
			}
			else
				_tprintf(TEXT(" |\t %s \t\t|"), empresasBoard[i].nomeEmpresa);
		}
		else {
			_tprintf(TEXT(" |\t %s \t|"), empresasBoard[i].nomeEmpresa);

		}

		_tprintf(TEXT(" |\t %lu \t\t|"), empresasBoard[i].nA��es);
		//contar os digitos para retificar espa�amento do Pre�o A��o
		digitos = 0;
		numeros = empresasBoard[i].pA��o;
		while (numeros >= 1) {
			numeros /= 10;
			digitos++;
		}
		if (digitos >= 2)
			_tprintf(TEXT(" |\t %.2f� \t|"), empresasBoard[i].pA��o);
		else
			_tprintf(TEXT(" |\t %.2f� \t\t|"), empresasBoard[i].pA��o);
		_tprintf(TEXT("\n\t\t---------------------------------------------------------------------------------\n"));
		
	}


	//MOSTRAR A ULTIMA TRANSA��O//
	_tprintf(TEXT("\n\t\t--------------------------------ULTIMA TRANSA��O---------------------------------\n"));
	_tprintf(TEXT("\n\t\t\t|\t NOME\t\t| |\t Num_A��es\t| |\t Pre�o-A��o\t|\n"));
	_tprintf(TEXT("\t\t---------------------------------------------------------------------------------\n"));
	
	if (_tcslen(boardDt->ultmTransacao->EmpresaNome) <= 5) {
		if (_tcsicmp(boardDt->ultmTransacao->EmpresaNome, TEXT("-1")) == 0) {
			_tprintf(TEXT(" |\t   \t\t|"));
			//_tprintf(TEXT("\t\t\t|\t %s\t\t|"), boardDt->ultmTransacao->EmpresaNome);
		}else {
			_tprintf(TEXT("\t\t\t|\t %s\t\t|"), boardDt->ultmTransacao->EmpresaNome);
		}
	}
	else {
		_tprintf(TEXT("\t\t\t|\t %s\t|"), boardDt->ultmTransacao->EmpresaNome);

	}
	_tprintf(TEXT(" | \t % lu\t\t|"), boardDt->ultmTransacao->nAcoes);



	//contar os digitos para retificar espa�amento do Pre�o A��o
	digitosT = 0;
	numerosT = boardDt->ultmTransacao->pAcao;
	while (numerosT >= 1) {
		numerosT /= 10;
		digitosT++;
	}
	if (digitosT >= 2)
		_tprintf(TEXT(" |\t %.2f� \t|"), boardDt->ultmTransacao->pAcao);
	else
		_tprintf(TEXT(" |\t %.2f� \t\t|"), boardDt->ultmTransacao->pAcao);




	/*
		if (_tcsicmp(boardDt->ultmTransacao->EmpresaNome,TEXT("-1")) != 0) {
			_tprintf(TEXT("\t\t\t|\t %s\t\t| |\t %lu\t\t| "), boardDt->ultmTransacao->EmpresaNome,
				boardDt->ultmTransacao->nAcoes);
			_tprintf(TEXT("|\t %.2f �\t|\n"), boardDt->ultmTransacao->pAcao);
		}
		*/
	

	_tprintf(TEXT("\n\t\t\t close - Fechar o programa\n"));
}
