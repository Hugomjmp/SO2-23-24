#include "..\\utils.h"

int _tmain(int argc, TCHAR* argv[])
{
    //clienteData CD;
    TCHAR login[TAM], password[TAM], comando[200];

    //variaveis
    BOOL resultado;
    DWORD nBytes;

    //HANDLES
    HANDLE hPipe;
    HANDLE hSemaphore;

    //Nome do namedpipe
    LPCTSTR pBolsa = TEXT("\\\\.\\pipe\\BOLSA");

    //Codigo para o unicode
#ifdef UNICODE
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    //_setmode(_fileno(stderr), _O_WTEXT);
#endif
    _tprintf(TEXT("#################################################################\n"));
    _tprintf(TEXT("#\t\t\t\t\t\t\t\t#\n"));
    _tprintf(TEXT("#\t\tBOLSA DE VALORES ONLINE\t\t\t\t#\n"));
    _tprintf(TEXT("#\t\t\t\t\t\t\t\t#\n"));
    _tprintf(TEXT("#################################################################\n"));

    // Inicializacao do semaforo
    hSemaphore = CreateSemaphore(NULL, 0, 1, TEXT("SEMAFORO"));
    if (hSemaphore == NULL) {
        _tprintf(TEXT("CreateSemaphore error: %d\n"), GetLastError());
        return 1;
    }

    //CREDENCIAIS DO UTILIZADOR
    _tprintf(TEXT("\nLogin: "));
    _tscanf(TEXT("%s"), &login);
    _tprintf(TEXT("\nPassword: "));
    _tscanf(TEXT("%s"), &password);
    // DEBUG...
    _tprintf(TEXT("\nLi -> Login: %s com tamanho %d e Password: %s com tamanho %zu"), login, _tcslen(login), password, _tcslen(password));

    //COMANDOS DO CLIENTE
    while ((_tcsicmp(TEXT("exit"), comando)) != 0)
    {
        _tprintf(TEXT("\nComando: "));
        _tscanf(TEXT("%s"), &comando);
        _tprintf(TEXT("\nLi -> Comando: %s com tamanho %zu"), comando, _tcslen(comando));
        /*resultado = WriteFile(
            hPipe,
            comando,
            (_tcslen(comando) + 1) * sizeof(TCHAR),
            &nBytes,
            NULL
        );*/
        //comando[_tcslen(comando) - 1] = '\0'; //retirar o /0

        if (!ReleaseSemaphore(
            hSemaphore,  
            1,           // Incrementar para 1 o semáforo quando este tem vaga
            NULL))       
        {
            _tprintf(TEXT("ReleaseSemaphore error: %d\n"), GetLastError());
        }
    }

    //CloseHandle(hPipe);

    CloseHandle(hSemaphore);

    return 0;
}


