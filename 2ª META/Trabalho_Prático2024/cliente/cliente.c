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

    //###############################################################
    //#																#
    //#							Threads								#
    //#																#
    //###############################################################
    /*hThread = NULL; // é bom inicializar a zero para depois podermos testar se a thread foi criada com sucesso
    hThread = CreateThread(
        NULL,					// default security attributes
        0,						// use default stack size
        cliente_read,			// thread function name
        NULL,				// argument to thread function
        0,						// use default creation flags
        NULL);
    if (hThread == NULL) {
        _tprintf(TEXT("Erro a criar a thread. Código de erro: %d\n", GetLastError()));
        return 1;
    }*/
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

        _tprintf(TEXT("\n[CLIENTE] Enviei %d bytes ao leitor %d... (WriteFile)\n"), n, i);
    } while (_tcscmp(cliData.RESPOSTA, TEXT("1")) == 0);
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

        _tprintf(TEXT("\n[CLIENTE] Enviei %d bytes ao leitor %d... (WriteFile)\n"), n, i);
    } while (_tcscmp(cliData.RESPOSTA, TEXT("exit")) != 0);

    return 0;
}


//###############################################################
//#																#
//#				Thread vai receber os dados do Bolsa			#
//#																#
//###############################################################
//TRATA DE RECEBER DADOS DO NAMEDPIPE
DWORD WINAPI cliente_read(LPVOID lparam) {
    //DWORD resultado, nBytes;
    HANDLE hPipe = (HANDLE)lparam;
    //clienteData cliData;

    //resultado = ReadFile(
    //    hPipe,        // handle to pipe 
    //    &cliData,   // buffer to receive data 
    //    sizeof(clienteData), // size of buffer 
    //    &nBytes, // number of bytes read 
    //    NULL);        // not overlapped I/O 
    //_tprintf(TEXT("\nResposta: %s "), cliData.comando);*/
}

DWORD WINAPI recebeMSG(LPVOID data) {

    DATA* ptd = (DATA*)data;
    HANDLE hPipe = ptd->hPipe;
    TCHAR buf[256];
    DWORD n;
    BOOL ret;
    clienteData cliData;
    OVERLAPPED ov;
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
        //buf[n / sizeof(TCHAR)] = _T('\0');
        _tprintf(TEXT("[LEITOR] Recebi %d bytes: '%s' '%s' '%s'... (ReadFile)\n"),n, cliData.login, cliData.password,cliData.RESPOSTA);

    } while (_tcscmp(cliData.RESPOSTA, TEXT("SAIR")) != 0);

    return 0;
}





void apresentacao() {
    _tprintf(TEXT("\t\t\t#################################################################\n"));
    _tprintf(TEXT("\t\t\t#\t\t\t\t\t\t\t\t#\n"));
    _tprintf(TEXT("\t\t\t#\t\tBOLSA DE VALORES ONLINE\t\t\t\t#\n"));
    _tprintf(TEXT("\t\t\t#\t\t\t\t\t\t\t\t#\n"));
    _tprintf(TEXT("\t\t\t#################################################################\n"));
}