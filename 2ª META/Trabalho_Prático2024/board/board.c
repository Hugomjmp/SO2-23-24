#include "..\\utils.h"

int _tmain(int argc, TCHAR* argv[])
{
	empresaData* pEmpresas;
	//empresaData(*eD)[MAX_COLUNA];
	HANDLE hEvent, hMapFile;

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
		FILE_ALL_ACCESS,	// acesso pretendido
		FALSE,
		SHM_NAME			// nome dado ao recurso (ficheiro mapeado)
	);
	/*if (hMapFile == NULL) {
		_tprintf(TEXT("Error: CreateFileMapping (%d)\n"), GetLastError());
		//return 1;
	}*/
	
	pEmpresas = (empresaData*)MapViewOfFile(
		hMapFile,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		sizeof(empresaData)
	);
	
	if (eD == NULL) {
		_tprintf(TEXT("ERROR: MapViewOfFile (%d)\n"), GetLastError());
		
		//return 1;
	}*/
	
	//empresaData empresaLida;

	while (1)
	{
		_tprintf(TEXT("\nANTES DO WAIT!!!!"));
		WaitForSingleObject(hEvent, INFINITE);
		_tprintf(TEXT("\nPASSEI O WAIT!!!!"));
		
		_tprintf(TEXT("Nome da Empresa: %s\n"), pEmpresas[0].nomeEmpresa);
		_tprintf(TEXT("Número de Ações: %lu\n"), pEmpresas[0].nAções);
		_tprintf(TEXT("Preço da Ação: %.2f\n"), pEmpresas[0].pAção);
			

		
	}
	UnmapViewOfFile(pEmpresas);
	CloseHandle(hMapFile);
	return 0;
}