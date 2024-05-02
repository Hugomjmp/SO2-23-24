#include "..\\utils.h"


//###############################################################
//#																#
//#				Main vai tratar da escrita do Cliente			#
//#																#
//###############################################################


int _tmain(int argc, TCHAR* argv[])
{
    //clienteData CD;
    TCHAR login[TAM], password[TAM], comando[200];
    TCHAR string[200];

    //variaveis
    BOOL resultado, writeResult;
    DWORD estado = 0;
    DWORD nBytes;
    DWORD clienteValido = 0;
    //HANDLES

    HANDLE hSemaphore;

    HANDLE hEvent;

    //Nome do namedpipe
    LPCTSTR pBolsa = TEXT("\\\\.\\pipe\\BOLSA");

    //ESTRUTURAS
    clienteData cliData;
    
    //Codigo para o unicode
#ifdef UNICODE
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    //_setmode(_fileno(stderr), _O_WTEXT);
#endif

    apresentacao();

    TCHAR buf[256];
    HANDLE hPipe;
    int i = 0;
    BOOL ret;
    DWORD n;
    HANDLE hThread;
    DATA data;

    


    //_tprintf(TEXT("[LEITOR] Esperar pelo pipe '%s' (WaitNamedPipe)\n"), NAME_PIPE);
    if (!WaitNamedPipe(NAME_PIPE, NMPWAIT_WAIT_FOREVER)) {
        _tprintf(TEXT("[ERRO] Ligar ao pipe '%s'! (WaitNamedPipe)\n"), NAME_PIPE);
        exit(-1);
    }
    //_tprintf(TEXT("\n[CLIENTE] Ligação ao pipe do escritor... (CreateFile)\n"));
    hPipe = CreateFile(NAME_PIPE, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
    if (hPipe == NULL) {
        _tprintf(TEXT("[ERRO] Ligar ao pipe '%s'! (CreateFile)\n"), NAME_PIPE);
        exit(-1);
    }
    //_tprintf(TEXT("[CLIENTE] Liguei-me...\n"));

    data.hPipe = hPipe;
    data.continua = TRUE;
    hThread = CreateThread(NULL, 0, recebeMSG, &data, 0, NULL);
    
    OVERLAPPED ov;
    HANDLE hEv = CreateEvent(NULL, TRUE, FALSE, NULL);
    do {
        //LER TECLADO...
        //ENVIAR PARA SERVIDOR...
        _tprintf(TEXT("\n\t\t\tLogin: "));
        _tscanf(TEXT("%s"), cliData.login);
        _tprintf(TEXT("\n\t\t\tPassword: "));
        _tscanf(TEXT("%s"), cliData.password);
        //buf[_tcslen(buf) - 1] = _T('\0');
        
        ZeroMemory(&ov, sizeof(ov));
        ov.hEvent = hEv;
        
        //ret = WriteFile(hPipe, buf, (DWORD)_tcslen(buf) * sizeof(TCHAR), &n, &ov);
        ret = WriteFile(hPipe, &cliData, sizeof(clienteData), &n, &ov);
        if (ret == TRUE) {
            _tprintf(TEXT("Escrevi de imediato\n"));
        }
        else {
            if (GetLastError() == ERROR_IO_PENDING) {
                _tprintf(TEXT("Agendei a escrita\n"));
                WaitForSingleObject(hEv, INFINITE);
                GetOverlappedResult(hPipe, &ov, &n, FALSE);
            }
            else {
                _tprintf(TEXT("[ERRO] FALHOU O OVERLAPPED\n"));
                break;
            }
        }


        //_tprintf(TEXT("\n[CLIENTE] Enviei %d bytes ao leitor %d... (WriteFile)\n"), n, i);
    } while (_tcsicmp(cliData.RESPOSTA, TEXT("1")) == 0);
    //TRATA DOS COMANDOS...
    
    do {
        //LER TECLADO...
        //ENVIAR PARA SERVIDOR...
        
        _tprintf(TEXT("\n\t\t\tComando: "));
        //_tscanf(TEXT("%s"), &cliData.comando);
        _fgetts(cliData.comando, 300, stdin);
        cliData.comando[_tcslen(cliData.comando) - 1] = _T('\0');
        ZeroMemory(&ov, sizeof(ov));
        ov.hEvent = hEv;

        //ret = WriteFile(hPipe, buf, (DWORD)_tcslen(buf) * sizeof(TCHAR), &n, &ov);
        ret = WriteFile(hPipe, &cliData, sizeof(clienteData), &n, &ov);
        
        if (ret == TRUE) {
            //_tprintf(TEXT("Escrevi de imediato\n"));
        }
        else {
            if (GetLastError() == ERROR_IO_PENDING) {
                _tprintf(TEXT("Agendei a escrita\n"));
                WaitForSingleObject(hEv, INFINITE);
                GetOverlappedResult(hPipe, &ov, &n, FALSE);
            }
            else {
                _tprintf(TEXT("[ERRO] FALHOU O OVERLAPPED\n"));
                break;
            }
        }

        //_tprintf(TEXT("\n[CLIENTE] Enviei %d bytes ao leitor %d... (WriteFile)\n"), n, i);
    } while (_tcscmp(cliData.RESPOSTA, TEXT("exit")) != 0);
    
    return 0;
}


//###############################################################
//#																#
//#				Thread vai receber os dados do Bolsa			#
//#																#
//###############################################################
DWORD WINAPI recebeMSG(LPVOID data) {
   
    DATA* ptd = (DATA*)data;
    HANDLE hPipe = ptd->hPipe;
    TCHAR buf[256];
    DWORD n;
    BOOL ret;
    clienteData cliData;
    OVERLAPPED ov;
    BOOL VERIFIÇÃO = FALSE;
    HANDLE hEv = CreateEvent(NULL, TRUE, FALSE, NULL);

    do {
        //RECEBER RESPOSTA...
        ZeroMemory(&ov, sizeof(ov));
        ov.hEvent = hEv;
        //ret = ReadFile(hPipe, buf, sizeof(buf), &n, &ov);
        
        ret = ReadFile(hPipe, &cliData, sizeof(clienteData), &n, &ov);
        
        if (ret == TRUE) {
           // _tprintf(TEXT("Li de imediato\n"));
        }
        else {
            if (GetLastError() == ERROR_IO_PENDING) {
                //_tprintf(TEXT("Agendei a leitura\n"));
                WaitForSingleObject(hEv, INFINITE);
                GetOverlappedResult(hPipe, &ov, &n, FALSE);
            }
            else {
                _tprintf(TEXT("[ERRO] FALHOU O OVERLAPPED\n"));
                break;
            }
        }
        if (_tcscmp(cliData.RESPOSTA, TEXT("1")) == 0) {

        }
        else if (_tcscmp(cliData.RESPOSTA, TEXT("-1")) == 0) {
            _tprintf(TEXT("\n\t\t\t Credenciais erradas!"));
        }
        else if (_tcscmp(cliData.RESPOSTA, TEXT("2")) == 0){
            _tprintf(TEXT("\n\t\t\t %s"), cliData.RESPOSTA);
            //mostra_tabela(&data);
        }
        //buf[n / sizeof(TCHAR)] = _T('\0');
        wprintf(L"%s", cliData.RESPOSTA);
       // _tprintf(TEXT("\n\t\t\t %s"), cliData.RESPOSTA);

    } while (_tcscmp(cliData.RESPOSTA, TEXT("SAIR")) != 0);

    return 0;
}


void mostra_tabela(LPVOID data) {
    DATA* ptd = (DATA*)data;
    DWORD contador = 0;
    DWORD digitos = 0;
    float numeros = 0.0;

	_tprintf(TEXT("\n\t\t| ID | |\t NOME\t\t| |\t Num_Ações\t| |\t Preço-Ação\t|\n"));
	_tprintf(TEXT("\t\t---------------------------------------------------------------------------------\n"));
	for (DWORD i = 0; i < MAX_EMPRESAS; i++) { //conta quantas empresas estão na tabela
        //_tprintf(TEXT(" |\t %s \t\t|"), ptd->clidData->empresas[i].nomeEmpresa);
		/*if (_tcsicmp(TEXT("-1"), ptd->clidData->emp[i].nomeEmpresa) != 0)
		{
			contador++;
		}*/

	}/*
	for (DWORD i = 0; i < contador; i++)
	{
		if (i < 9) { //corrige o espaçamento do ID
			_tprintf(TEXT("\t\t| %d  |"), i + 1);
		}
		else {
			_tprintf(TEXT("\t\t| %d |"), i + 1);
		}
		if (_tcslen(ptd->clidData->emp[i].nomeEmpresa) <= 5)
		{
            if (_tcscmp(ptd->clidData->emp[i].nomeEmpresa, TEXT("-1")) == 0) {
				_tprintf(TEXT(" |\t   \t\t|"), ptd->clidData->emp[i].nomeEmpresa); // coloca esppaços vazios onde está "-1"
			}
			else
				_tprintf(TEXT(" |\t %s \t\t|"), ptd->clidData->emp[i].nomeEmpresa);
		}
		else {
			_tprintf(TEXT(" |\t %s \t|"), ptd->clidData->emp[i].nomeEmpresa);

		}
		_tprintf(TEXT(" |\t %lu \t\t|"), ptd->clidData->emp[i].nAções);
		//contar os digitos para retificar espaçamento do Preço Ação
		digitos = 0;
		numeros = ptd->clidData->emp[i].pAção;
		while (numeros >= 1) {
			numeros /= 10;
			digitos++;
		}
		if (digitos >= 2)
			_tprintf(TEXT(" |\t %.2f€ \t|"), ptd->clidData->emp[i].pAção);
		else
			_tprintf(TEXT(" |\t %.2f€ \t\t|"), ptd->clidData->emp[i].pAção);

        _tprintf(TEXT("\n\t\t---------------------------------------------------------------------------------\n"));
    }
    contador = 0;*/
}

void apresentacao() {
    _tprintf(TEXT("\t\t\t#################################################################\n"));
    _tprintf(TEXT("\t\t\t#\t\t\t\t\t\t\t\t#\n"));
    _tprintf(TEXT("\t\t\t#\t\tBOLSA DE VALORES ONLINE\t\t\t\t#\n"));
    _tprintf(TEXT("\t\t\t#\t\t\t\t\t\t\t\t#\n"));
    _tprintf(TEXT("\t\t\t#################################################################\n"));
}