#include "..\\utils.h"

int _tmain(int argc, TCHAR* argv[])
{
	HANDLE hEvent, hMapFile, hMutex;
	empresaData* emP;
	empresaData empresasBoard[30];
	
	DWORD contador = 0, numeros;
	DWORD digitos = 0;
#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	hEvent = OpenEvent(
		SYNCHRONIZE,	//dwDesiredAccess,
		FALSE,			//bInheritHandle,
		EVENT_NAME		//lpName
	);
	if (hEvent == NULL) {
		_tprintf(TEXT("Erro ao abrir o evento. Código de erro: %d\n", GetLastError()));
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
	
	hMutex = OpenMutex(
		MUTEX_ALL_ACCESS,
		FALSE,
		MUTEX_NAME);

	if (hMutex == NULL) {
		_tprintf(TEXT("ERROR: Mutex (%d)\n"), GetLastError());
		return 1;
	}
		while (1)
	{
		_tprintf(TEXT("\nANTES DO WAIT!!!!"));
		WaitForSingleObject(hEvent, INFINITE);
		WaitForSingleObject(hMutex, INFINITE);
		for (DWORD i = 0; i < MAX_EMPRESAS; i++) //copia o que tem na shared memory para um array local
		{
			empresasBoard[i] = emP[i];
		}
		ReleaseMutex(hMutex);
		_tprintf(TEXT("\nPASSEI O WAIT!!!!"));


		//REORGANIZAÇÃO....
		
		for (int i = 0; i < MAX_EMPRESAS - 1; i++) {
			for (int j = 0; j < MAX_EMPRESAS - i - 1; j++) {
				if (empresasBoard[j].pAção < empresasBoard[j + 1].pAção) {
					empresaData temp = empresasBoard[j];
					empresasBoard[j] = empresasBoard[j + 1];
					empresasBoard[j + 1] = temp;
				}
			}
		}

		//--------------
		_tprintf(TEXT("\n------------------------------REORGANIZADO---------------------------------------\n"));
		_tprintf(TEXT("\n| ID | |\t NOME\t\t| |\t Num_Ações\t| |\t Preço-Ação\t|\n"));
		_tprintf(TEXT("---------------------------------------------------------------------------------\n"));
		for (DWORD i = 0; i < MAX_EMPRESAS; i++)
		{
			if (i < 9) { //corrige o espaçamento do ID
				_tprintf(TEXT("| %d  |"), i + 1);
			}
			else {
				_tprintf(TEXT("| %d |"), i + 1);
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
			_tprintf(TEXT("\n---------------------------------------------------------------------------------\n"));
		}
	}
	UnmapViewOfFile(emP);
	CloseHandle(hMapFile);
	return 0;
}

