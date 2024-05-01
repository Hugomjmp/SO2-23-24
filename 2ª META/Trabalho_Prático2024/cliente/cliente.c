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
    HANDLE hPipe;
    HANDLE hSemaphore;
    HANDLE hThread;
    HANDLE hEvent;

    //Nome do namedpipe
    LPCTSTR pBolsa = TEXT("\\\\.\\pipe\\BOLSA");

    //ESTRUTURAS
    clienteData cliData;

    OVERLAPPED ov;
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
    hThread = NULL; // é bom inicializar a zero para depois podermos testar se a thread foi criada com sucesso
    hThread = CreateThread(
        NULL,					// default security attributes
        0,						// use default stack size
        cliente_read,			// thread function name
        /*hPipe*/NULL,				// argument to thread function
        0,						// use default creation flags
        NULL);
    if (hThread == NULL) {
        _tprintf(TEXT("Erro a criar a thread. Código de erro: %d\n", GetLastError()));
        return 1;
    }
    

    

    //---------------------------------------------------------------
    //###############################################################
    //#																#
    //#							Evento								#
    //#																#
    //###############################################################
    hEvent = CreateEvent(
        NULL,			//lpEventAttributes
        TRUE,			//bManualReset
        FALSE,			//bInitialState
        NULL	        //lpName
    );
    if (hEvent == NULL) {
        _tprintf(TEXT("Erro ao abrir o evento. Código de erro: %d\n", GetLastError()));
        return 1;
    }

    // Inicializacao do semaforo
   /* hSemaphore = CreateSemaphore(NULL, 0, 1, TEXT("SEMAFORO"));
    if (hSemaphore == NULL) {
        _tprintf(TEXT("CreateSemaphore error: %d\n"), GetLastError());
        return 1;
    }*/




    _tprintf(TEXT("[LEITOR] Esperar pelo pipe '%s' (WaitNamedPipe)\n"),
        NAME_PIPE);
    if (!WaitNamedPipe(NAME_PIPE, NMPWAIT_WAIT_FOREVER)) {
        _tprintf(TEXT("[ERRO] Ligar ao pipe '%s'! (WaitNamedPipe)\n"), NAME_PIPE);
        exit(-1);
    }
    //###############################################################
//#																#
//#							Pipe								#
//#																#
//###############################################################
    hPipe = CreateFile(
        NAME_PIPE,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,
        NULL
    );
    if (hPipe == NULL) {
        _tprintf(TEXT("[ERRO] Ligar ao pipe '%s'! (CreateFile)\n"), NAME_PIPE);
        return -1;
    }

    //---------------------------------------------------------------
    //CREDENCIAIS DO UTILIZADOR
    while (1)
    {
        if(clienteValido == 0){
        _tprintf(TEXT("\nLogin: "));
        _tscanf(TEXT("%s"), &cliData.login);
        _tprintf(TEXT("\nPassword: "));
        _tscanf(TEXT("%s"), &cliData.password);

        ZeroMemory(&ov, sizeof(OVERLAPPED));

        writeResult = WriteFile(
            hPipe,
            &cliData,
            sizeof(cliData),
            &nBytes,
            &ov
        );

        FlushFileBuffers(hPipe);
        //SetEvent(hEvent);
        //Sleep(500);
        //ResetEvent(hEvent);

        }
        ZeroMemory(&ov, sizeof(ov));
        ov.hEvent = hEvent;
        resultado = ReadFile(
            hPipe,                  // handle to pipe 
            &cliData,               // buffer to receive data 
            sizeof(clienteData),    // size of buffer 
            &nBytes,                // number of bytes read 
            &ov);                   // overlapped I/O */
        _tprintf(TEXT("\nresultado %lu "), resultado);
        if (resultado == TRUE) {
            
        }
        else {
            if (GetLastError() == ERROR_IO_PENDING) {
                //_tprintf(TEXT("Agendei a leitura\n"));
                WaitForSingleObject(hEvent, INFINITE);
                GetOverlappedResult(hPipe, &ov, &nBytes, FALSE);
                _tprintf(TEXT("\nResposta Login : %s"), cliData.RESPOSTA);
                if (_tcsicmp(cliData.RESPOSTA, TEXT("1")) == 0) {
                    clienteValido = 1;
                    _tprintf(TEXT("\nComando: "));
                    _tscanf(TEXT("%s"), &cliData.comando);
                    ZeroMemory(&ov, sizeof(OVERLAPPED));
                    writeResult = WriteFile(
                        hPipe,
                        &cliData,
                        sizeof(cliData),
                        &nBytes,
                        &ov
                    );
                }
            }
            else {
                _tprintf(TEXT("[ERRO] FALHOU O OVERLAPPED\n"));

            }
        }
    }
        //    }
        //}
    //    _tprintf(TEXT("O cliente %s não existe ou tem as creedenciais erradas!"), cliData.RESPOSTA);


    //
    // DEBUG...
    //_tprintf(TEXT("\nLi -> Login: %s com tamanho %d e Password: %s com tamanho %zu"), 
    //    cliData.login, _tcslen(cliData.login), cliData.password, _tcslen(cliData.password));

    //COMANDOS DO CLIENTE
    //while ((_tcsicmp(TEXT("exit"), cliData.comando)) != 0)
    //{
    //    _tprintf(TEXT("\nComando: "));
    //    _tscanf(TEXT("%s"), cliData.comando);
    //    _tprintf(TEXT("\nLi -> Comando: %s com tamanho %zu"), cliData.comando, _tcslen(comando));
    //    //writeResult = WriteFile(
    //    //    hPipe,
    //    //    &cliData,
    //    //    sizeof(clienteData),
    //    //    &nBytes,
    //    //    NULL
    //    //);
    //    //FlushFileBuffers(hPipe);

    //    //comando[_tcslen(comando) - 1] = '\0'; //retirar o /0
    //    /*
    //    if (!ReleaseSemaphore(
    //        hSemaphore,
    //        1,           // Incrementar para 1 o semáforo quando este tem vaga
    //        NULL))
    //    {
    //        _tprintf(TEXT("ReleaseSemaphore error: %d\n"), GetLastError());
    //    }*/
    //}

    //CloseHandle(hPipe);

    //CloseHandle(hSemaphore);

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

void apresentacao() {
    _tprintf(TEXT("#################################################################\n"));
    _tprintf(TEXT("#\t\t\t\t\t\t\t\t#\n"));
    _tprintf(TEXT("#\t\tBOLSA DE VALORES ONLINE\t\t\t\t#\n"));
    _tprintf(TEXT("#\t\t\t\t\t\t\t\t#\n"));
    _tprintf(TEXT("#################################################################\n"));
}