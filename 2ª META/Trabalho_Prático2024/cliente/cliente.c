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

   

    //Nome do namedpipe
    LPCTSTR pBolsa = TEXT("\\\\.\\pipe\\BOLSA");

    //ESTRUTURAS
    clienteData cliData;
    clienteResposta cliRes;
    
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
    BOOL retR;
    DWORD n;
    HANDLE hThread;
    DATA data;

    
    HANDLE hSemClientes = OpenSemaphore(
        SEMAPHORE_ALL_ACCESS,
        FALSE,
        SEM_CLIENT_NAME);


    //_tprintf(TEXT("[LEITOR] Esperar pelo pipe '%s' (WaitNamedPipe)\n"), NAME_PIPE);
    if (!WaitNamedPipe(NAME_PIPE, NMPWAIT_WAIT_FOREVER)) {
        avisos(2);
        //_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'! (WaitNamedPipe)\n"), NAME_PIPE);
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
    
    OVERLAPPED ov;
    HANDLE hEv = CreateEvent(NULL, TRUE, FALSE, NULL);
    HANDLE hEvent = OpenEvent(
        SYNCHRONIZE,	//dwDesiredAccess,
        FALSE,			//bInheritHandle,
        EVENT_NAME_V		//lpName
    );
    do { 
        //LER TECLADO...
        //ENVIAR PARA SERVIDOR...
        _tprintf(TEXT("\n\t\t\tLogin: "));
        _fgetts(cliData.login, sizeof(cliData.login), stdin);
        cliData.login[_tcslen(cliData.login) - 1] = _T('\0');
        _tprintf(TEXT("\n\t\t\tPassword: "));
        _fgetts(cliData.password, sizeof(cliData.password), stdin);
        cliData.password[_tcslen(cliData.password) - 1] = _T('\0');
        
        ZeroMemory(&ov, sizeof(ov));
        ov.hEvent = hEv;

        ret = WriteFile(hPipe, &cliData, sizeof(clienteData), &n, &ov);
        if (ret == TRUE) {
            //_tprintf(TEXT("Escrevi de imediato\n"));
        }
        else {
            if (GetLastError() == ERROR_IO_PENDING) {
                //_tprintf(TEXT("Agendei a escrita\n"));
                WaitForSingleObject(hEv, INFINITE);
                GetOverlappedResult(hPipe, &ov, &n, FALSE);
            }
            else {
                _tprintf(TEXT("[ERRO] FALHOU O OVERLAPPED\n"));
                break;
            }
        }

        retR = ReadFile(hPipe, &cliRes, sizeof(clienteResposta), &n, &ov);
        
        if (retR == TRUE) {
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
    if (_tcscmp(cliRes.RESPOSTA, TEXT("-1")) == 0) {
        _tprintf(TEXT("\n\t\t\tCredenciais erradas!\n"));
    }
    else if (_tcscmp(cliRes.RESPOSTA, TEXT("3")) == 0) {

        avisos(1);
    }

        Sleep(100);

    } while (_tcsicmp(cliRes.RESPOSTA, TEXT("1")) != 0);
    WaitForSingleObject(hSemClientes, INFINITE);
    hThread = CreateThread(NULL, 0, recebeMSG, &data, 0, NULL);
    //TRATA DOS COMANDOS...
    do {
        //LER TECLADO...
        //ENVIAR PARA SERVIDOR...
        fflush(stdin);
        Sleep(200);
        ZeroMemory(&ov, sizeof(ov));
        ov.hEvent = hEv;
        
        _tprintf(TEXT("\n\t\t\tComando: "));

        _fgetts(cliData.comando, sizeof(cliData.comando), stdin);
        cliData.comando[_tcslen(cliData.comando) - 1] = _T('\0');
        



        ret = WriteFile(hPipe, &cliData, sizeof(clienteData), &n, &ov);

        if (ret == TRUE) {
            //_tprintf(TEXT("Escrevi de imediato\n"));
        }
        else {
            if (GetLastError() == ERROR_IO_PENDING) {
                //_tprintf(TEXT("Agendei a escrita\n"));
                WaitForSingleObject(hEv, INFINITE);
                GetOverlappedResult(hPipe, &ov, &n, FALSE);
            }
            else {
                _tprintf(TEXT("[ERRO] FALHOU O OVERLAPPED\n"));
                break;
            }
        }

    } while (_tcscmp(cliRes.RESPOSTA, TEXT("exit")) != 0);
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
    clienteResposta cliRes;
    empresaData empresas[30];
    OVERLAPPED ov;
    BOOL VERIFIÇÃO = FALSE;
    HANDLE hEv = CreateEvent(NULL, TRUE, FALSE, NULL);
    HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, EVENT_NAME_V);



    do {
       
        mostraMenuCliente();
        //RECEBER RESPOSTA...
        ZeroMemory(&ov, sizeof(ov));
        ov.hEvent = hEv;
        //ret = ReadFile(hPipe, buf, sizeof(buf), &n, &ov);
        
        ret = ReadFile(hPipe, &cliRes, sizeof(clienteResposta), &n, &ov);
        
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
                _tprintf(TEXT("[ERRO] O SERVIDOR BOLSA FOI OFFLINE\n"));
                _tprintf(TEXT("O programa Cliente vai ser desligado...\n"));
                break;
            }
        }
        if (_tcscmp(cliRes.RESPOSTA, TEXT("1")) == 0) {
            _tprintf(TEXT("Login efetuado com sucesso!\n"));
        } 
        //mostra o resultado do listc
        else if (_tcscmp(cliRes.RESPOSTA, TEXT("2")) == 0){ 
            DWORD contador = 0;
            DWORD digitos = 0;
            float numeros = 0.0;

            _tprintf(TEXT("\n\t\t| ID | |\t NOME\t\t| |\t Num_Ações\t| |\t Preço-Ação\t|\n"));
            _tprintf(TEXT("\t\t---------------------------------------------------------------------------------\n"));
            for (DWORD i = 0; i < MAX_EMPRESAS; i++) { //conta quantas empresas estão na tabela
                if (_tcsicmp(TEXT("-1"), cliRes.empW[i].nomeEmpresa) != 0)
                    contador++;
            }
            for (DWORD i = 0; i < contador; i++)
            {
                if (i < 9) { //corrige o espaçamento do ID
                    _tprintf(TEXT("\t\t| %d  |"), i + 1);
                }
                else {
                    _tprintf(TEXT("\t\t| %d |"), i + 1);
                }
                if (_tcslen(cliRes.empW[i].nomeEmpresa) <= 5)
                {
                    if (_tcscmp(cliRes.empW[i].nomeEmpresa, TEXT("-1")) == 0) {
                        _tprintf(TEXT(" |\t   \t\t|"), cliRes.empW[i].nomeEmpresa); // coloca esppaços vazios onde está "-1"
                    }
                    else
                        _tprintf(TEXT(" |\t %s \t\t|"), cliRes.empW[i].nomeEmpresa);
                }
                else {
                    _tprintf(TEXT(" |\t %s \t|"), cliRes.empW[i].nomeEmpresa);

                }
                _tprintf(TEXT(" |\t %lu \t\t|"), cliRes.empW[i].nAções);
                //contar os digitos para retificar espaçamento do Preço Ação
                digitos = 0;
                numeros = cliRes.empW[i].pAção;
                while (numeros >= 1) {
                    numeros /= 10;
                    digitos++;
                }
                if (digitos >= 2)
                    _tprintf(TEXT(" |\t %.2f€ \t|"), cliRes.empW[i].pAção);
                else
                    _tprintf(TEXT(" |\t %.2f€ \t\t|"), cliRes.empW[i].pAção);

                _tprintf(TEXT("\n\t\t---------------------------------------------------------------------------------\n"));
            }
            contador = 0;
        }
        else 
        {   
            _tprintf(TEXT("\n\t\t\t%s"), cliRes.RESPOSTA);
        }

       //SetEvent(hEvent);
       //Sleep(500);
       //ResetEvent(hEvent);
    } while (_tcscmp(cliRes.RESPOSTA, TEXT("SAIR")) != 0);

    return 0;
}




void apresentacao() {
    _tprintf(TEXT("\t\t\t\033[0;32m#################################################################\033[0m\n"));
    _tprintf(TEXT("\t\t\t\033[0;32m#\t\t\t\t\t\t\t\t\033[0;32m#\033[0m\n"));
    _tprintf(TEXT("\t\t\t\033[0;32m#\t\t\tBOLSA DE VALORES ONLINE\t\t\t\033[0;32m#\033[0m\n"));
    _tprintf(TEXT("\t\t\t\033[0;32m#\t\t\t\t\t\t\t\t\033[0;32m#\033[0m\n"));
    _tprintf(TEXT("\t\t\t\033[0;32m#################################################################\033[0m\n"));
}

void mostraMenuCliente() {
    _tprintf(TEXT("\n\t\t\t\033[0;32m+---------------------------------------------------------------+\033[0m"));
    _tprintf(TEXT("\n\t\t\t\033[0;32m|\t\t\t\tMENU\t\t\t\t\033[0;32m|\033[0m"));
    _tprintf(TEXT("\n\t\t\t\033[0;32m+---------------------------------------------------------------+\033[0m"));
    _tprintf(TEXT("\n\t\t\t\033[0;32m|\033[0m \033[1;33mlistc\033[0m   - Listar todas as empresas \t\t\t\t\033[0;32m|\033[0m"));
    _tprintf(TEXT("\n\t\t\t\033[0;32m|\033[0m \033[1;33mbuy\033[0m     - Comprar ações\t\t\t\t\t\033[0;32m|\033[0m"));
    _tprintf(TEXT("\n\t\t\t\033[0;32m|\033[0m \033[1;33msell\033[0m    - Vender ações\t\t\t\t\t\033[0;32m|\033[0m"));
    _tprintf(TEXT("\n\t\t\t\033[0;32m|\033[0m \033[1;33mbalance\033[0m - Consultar saldo \t\t\t\t\t\033[0;32m|\033[0m"));
    _tprintf(TEXT("\n\t\t\t\033[0;32m|\033[0m \033[1;33mexit\033[0m    - Sair da plataforma\t\t\t\t\t\033[0;32m|\033[0m"));
    _tprintf(TEXT("\n\t\t\t\033[0;32m+---------------------------------------------------------------+\033[0m\n"));
}

void avisos(int x) {
    if (x==1)
    {
        _tprintf(TEXT("\n\t\t\t\033[1;31m      /\\    "));
        _tprintf(TEXT("\n\t\t\t\033[1;31m     /  \\    "));
        _tprintf(TEXT("\n\t\t\t\033[1;31m    /\033[0m |\033[1;31m  \\   "));
        _tprintf(TEXT("\n\t\t\t\033[1;31m   /\033[0m  .\033[1;31m   \\  \033[0mJá tem uma sessão ativa com esse login!"));
        _tprintf(TEXT("\n\t\t\t\033[1;31m  /________\\ \033[0m\n"));
    }
    if (x == 2)
    {
        _tprintf(TEXT("\n\t\t\t\033[1;31m      /\\    "));
        _tprintf(TEXT("\n\t\t\t\033[1;31m     /  \\    "));
        _tprintf(TEXT("\n\t\t\t\033[1;31m    /\033[0m |\033[1;31m  \\   "));
        _tprintf(TEXT("\n\t\t\t\033[1;31m   /\033[0m  .\033[1;31m   \\  \033[0mO servidor não se encontra ONLINE!"));
        _tprintf(TEXT("\n\t\t\t\033[1;31m  /________\\ \033[0m\n"));
    }
}