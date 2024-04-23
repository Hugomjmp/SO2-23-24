#include "..\\Trabalho_Prático2024\utils.h"

int _tmain(int argc, TCHAR* argv[])
{
	//HANDLES...

	HANDLE hPipe, hThreadArray[5];

	//Variáveis
	DWORD nSegundos = 0, nAções = 0, linha, nBytes, numeros;
	TCHAR digitos = 0;
	
	float pAção;
	TCHAR comandoAdmin[200], nomeEmpresa[50], string[100];
	BOOL resultado;
	//TCHAR empresas[MAX_LINHA][MAX_COLUNA][50];
	
	//ficheiros
	FILE* fp,*fpU;

	//estruturas
	clienteData cd;
	userData user[5][MAX_COLUNA];
	empresaData empresas[MAX_LINHA][MAX_COLUNA];

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	//_setmode(_fileno(stderr), _O_WTEXT);
#endif
	//inicializar a matriz com -1, 0 , 0.0 em todas as linhas
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < MAX_COLUNA; j++) {
			_tcscpy(user[i][j].user, TEXT("-1"));
			user[i][j].saldo = 0;
			user[i][j].estado = 0;
		}
	}
	fpU = fopen(UTILIZADORES_REGISTADOS, "r"); //abertura do ficheiro Clientes.txt para leitura
	if (fpU == NULL) //verifica se o ficheiro existe
	{
		_tprintf(TEXT("[ERRO] ao abrir o ficheiro! \n"));
		return -1;
	}
	//_tprintf(TEXT("\nRecebi Comando: %s com tamanho %zu"), comandoAdmin, _tcslen(comandoAdmin));
	linha = 0;
	while (linha < 5 && fwscanf(fpU, TEXT("%s %*s %f"), user[linha][0].user, &user[linha][1].saldo) == 2) {
		linha++;
	}
	//inicializar a matriz com -1, 0 , 0.0 em todas as linhas
	for (int i = 0; i < MAX_LINHA; i++) {
		for (int j = 0; j < MAX_COLUNA; j++) {
			_tcscpy(empresas[i][j].nomeEmpresa, TEXT("-1"));
			empresas[i][j].nAções = 0;
			empresas[i][j].pAção = 0.0;
		}
	}
	CriaRegedit(); //TRATA DA VARIAVEL NCLIENTES
	//leRegedit();	//função para tirar depois
	escreveRegedit();//função para tirar depois

	//CIRAR A THREAD PARA TRATAR OS COMANDOS DOS CLIENTES
	/*hThreadArray[0] = NULL; // é bom inicializar a zero para depois podermos testar se a thread foi criada com sucesso
	hThreadArray[0] = CreateThread(
						NULL,					// default security attributes
						0,						// use default stack size
						trataComandosClientes,	// thread function name
						&hPipe,					// argument to thread function
						0,						// use default creation flags
						NULL);
						*/
	/*hThreadArray[1] = NULL; // é bom inicializar a zero para depois podermos testar se a thread foi criada com sucesso
	hThreadArray[1] = CreateThread(
		NULL,					// default security attributes
		0,						// use default stack size
		verificaClientes,	// thread function name
		NULL,					// argument to thread function
		0,						// use default creation flags
		NULL);*/
							//COMANDO DO ADMINISTRADOR
	while ((_tcsicmp(TEXT("close"), comandoAdmin)) != 0)
	{
		_tprintf(TEXT("\n addc - Acrescentar uma empresa "));
		_tprintf(TEXT("\n adde - Acrescentar empresas via Empresas.txt "));
		_tprintf(TEXT("\n listc - Listar todas as empresas "));
		_tprintf(TEXT("\n stock - Redefinir custo das ações de uma empresa "));
		_tprintf(TEXT("\n users - Listar utilizadores"));
		_tprintf(TEXT("\n pause - Pausar as operações de compra e venda"));
		_tprintf(TEXT("\n clear - Limpa o ecrã"));
		_tprintf(TEXT("\n close - Encerrar a plataforma"));
		_tprintf(TEXT("\nComando: "));
		fflush(stdin);
		_tscanf(TEXT("%s"), comandoAdmin);
		//_tprintf(TEXT("\nLi -> Comando: %s com tamanho %zu"), comandoAdmin, _tcslen(comandoAdmin));

		//TRATA DO COMANNDO ACRESCENTAR UM EMPRESA
		if (_tcsicmp(TEXT("addc"), comandoAdmin, _tcslen(TEXT("addc"))) == 0)
		{
			_tscanf(TEXT("%s"), &nomeEmpresa);
			_tscanf(TEXT("%lu"), &nAções);
			_tscanf(TEXT("%f"), &pAção);
			DWORD empresaAdiciona = 1;
			for (DWORD i = 0; i < MAX_LINHA; i++) {
				if (empresaAdiciona == 1) {
					if ((_tcsicmp(TEXT("-1"), empresas[i][0].nomeEmpresa)) == 0) {
						_tcscpy(empresas[i][0].nomeEmpresa, nomeEmpresa);
						empresas[i][1].nAções = nAções;
						empresas[i][2].pAção = pAção;
						empresaAdiciona = 0;
					}
				}
			}

			//_tprintf(TEXT("\nRecebi Comando: %s %s %lu %lu com tamanho %zu"), comandoAdmin,nomeEmpresa,nAções,pAção, _tcslen(comandoAdmin));
		}
		//TRATA DO COMANNDO ACRESCENTAR UM EMPRESA via file txt
		if (_tcsicmp(TEXT("adde"), comandoAdmin, _tcslen(TEXT("adde"))) == 0)
		{
			fp = fopen(EMPRESAS, "r"); //abertura do ficheiro Clientes.txt para leitura
			if (fp == NULL) //verifica se o ficheiro existe
			{
				_tprintf(TEXT("[ERRO] ao abrir o ficheiro! \n"));
				return -1;
			}
			//_tprintf(TEXT("\nRecebi Comando: %s com tamanho %zu"), comandoAdmin, _tcslen(comandoAdmin));
			linha = 0;
			while (linha < MAX_LINHA && fwscanf(fp, TEXT("%s %lu %f"), &empresas[linha][0].nomeEmpresa, &empresas[linha][1].nAções, &empresas[linha][2].pAção) == 3) {
				linha++;
			}


			fclose(fp);
		}
		//TRATA DO COMANDO LISTAR TODAS AS EMPRESAS
		else if (_tcsicmp(TEXT("listc"), comandoAdmin) == 0) {
			//_tprintf(TEXT("\nRecebi Comando: %s com tamanho %zu"), comandoAdmin, _tcslen(comandoAdmin));
			_tprintf(TEXT("\n| ID | |\t NOME\t\t| |\t Num_Ações\t| |\t Preço-Ação\t|\n"));
			_tprintf(TEXT("---------------------------------------------------------------------------------\n"));
			for (DWORD i = 0; i < MAX_LINHA; i++)
			{
				if (i < 9) { //corrige o espaçamento do ID
					_tprintf(TEXT("| %d  |"), i + 1);
				}else{
					_tprintf(TEXT("| %d |"), i + 1);
				}
					if (_tcslen(empresas[i][0].nomeEmpresa) <= 5)
					{
						if (_tcscmp(empresas[i][0].nomeEmpresa, TEXT("-1")) == 0) {
							_tprintf(TEXT(" |\t   \t\t|"), empresas[i][0].nomeEmpresa); // coloca esppaços vazios onde está "-1"
						}else
						_tprintf(TEXT(" |\t %s \t\t|"), empresas[i][0].nomeEmpresa);
					}
					else {
						_tprintf(TEXT(" |\t %s \t|"), empresas[i][0].nomeEmpresa);

					}			
					_tprintf(TEXT(" |\t %lu \t\t|"), empresas[i][1].nAções);
					//contar os digitos para retificar espaçamento do Preço Ação
					digitos = 0;
					numeros = empresas[i][2].pAção;
					while (numeros >= 1) { 
						numeros /= 10;
						digitos++;
					}
					if (digitos >= 2)
						_tprintf(TEXT(" |\t %.2f€ \t|"), empresas[i][2].pAção);
					else
						_tprintf(TEXT(" |\t %.2f€ \t\t|"), empresas[i][2].pAção);

				_tprintf(TEXT("\n---------------------------------------------------------------------------------\n"));
			}

		}
		//TRATA DO COMANDO REDEFINIR CUSTO DAS AÇÕES DE UMA EMPRESA
		else if (_tcsicmp(TEXT("stock"), comandoAdmin) == 0) {
			_tscanf(TEXT("%s"), &nomeEmpresa);
			_tscanf(TEXT("%f"), &pAção);

			for (DWORD i = 0; i < MAX_LINHA; i++) {
					if ((_tcsicmp(empresas[i][0].nomeEmpresa, nomeEmpresa)) == 0) {
						empresas[i][2].pAção = pAção;
					}
			}
		}
		//TRATA DO COMANDO LISTAR UTLIZADORES
		else if (_tcsicmp(TEXT("users"), comandoAdmin) == 0) {
			//_tprintf(TEXT("\nRecebi Comando: %s com tamanho %zu"), comandoAdmin, _tcslen(comandoAdmin));


			_tprintf(TEXT("\n| ID | |\t NOME\t\t| |\t Saldo \t\t| |\t Estado\t\t|\n"));
			_tprintf(TEXT("---------------------------------------------------------------------------------\n"));
			for (DWORD i = 0; i < 5; i++)
			{
				_tprintf(TEXT("| %d  |"), i + 1);

				_tprintf(TEXT(" |\t %s \t\t|"), user[i][0].user); 
				//_tprintf(TEXT(" |\t %.2f \t\t|"), user[i][2].saldo);
				digitos = 0;
				numeros = user[i][1].saldo;
				while (numeros >= 1) {
					numeros /= 10;
					digitos++;
				}
				
				if (digitos >= 3)
					_tprintf(TEXT(" |\t %.2f \t|"), user[i][1].saldo);
				else
					_tprintf(TEXT(" |\t %.2f \t\t|"), user[i][1].saldo);

				_tprintf(TEXT(" |\t %d \t\t|"), user[i][2].estado);

				_tprintf(TEXT("\n---------------------------------------------------------------------------------\n"));
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
			_tprintf(TEXT("\nRecebi Comando: %s %lu com tamanho %zu"), comandoAdmin, nSegundos, _tcslen(comandoAdmin));

		}
		//TRATA DA FALHA DO COMANDO
		else {
			if((_tcsicmp(TEXT("close"), comandoAdmin) != 0))
			_tprintf(TEXT("\nComando: %s introduzido com tamanho %zu, não existe!"), comandoAdmin, _tcslen(comandoAdmin));
		}

	}
	
	fclose(fpU);
	return 0;
}

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
	_tprintf(TEXT("NCLIENTES (%lu)\n"), valor);
	return valor;
}

void escreveRegedit() {
	HKEY hChaveClientes;
	LSTATUS resultado;
	DWORD tipo = REG_DWORD;
	DWORD tam = sizeof(DWORD);
	DWORD valor;
	valor = leRegedit();
	valor--;
	resultado = RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Trabalho_Pratico2024"), 0, KEY_SET_VALUE, &hChaveClientes); //abre a chave
	if (resultado != ERROR_SUCCESS)
	{
		_tprintf(TEXT("Erro ao abrir a chave (%d)\n"), GetLastError());
		return -1;
	}
	resultado = RegSetValueEx(hChaveClientes, TEXT("NCLIENTES"),0,REG_DWORD,(LPBYTE)&valor,tam);//le o conteudo
	if (resultado != ERROR_SUCCESS) {
		_tprintf(TEXT("Erro ao escrever o valor (%d)\n"), GetLastError());
		return -1;
	}
	
}
// Por acabar...
DWORD WINAPI trataComandosClientes() {
	
	clienteData cd;
	TCHAR buf[200];
	DWORD nbytes;
	HANDLE hPipe;
	_tprintf(TEXT("\nÀ espera de clientes"));
	//ConnectNamedPipe(hPipe, NULL);
	_tprintf(TEXT("\nRecebi cliente"));
	
	while ((_tcsicmp(TEXT("exit"), cd.comando)) != 0)
	//ReadFile(hPipe, cd.comando, 200 * sizeof(TCHAR), &nbytes, NULL);
	//TRATA DO COMANDO LISTAR TODAS AS EMPRESAS
	if (_tcsicmp(TEXT("listc"), cd.comando) == 0)
	{
		_tprintf(TEXT("\nRecebi do cliente o Comando: %s com tamanho %d"), cd.comando, _tcslen(cd.comando) - 1);
	}
	//TRATA DO COMANDO COMPRAR AÇÕES
	else if (_tcsicmp(TEXT("buy"), cd.comando) == 0) {
		_tprintf(TEXT("\nRecebi do cliente o Comando: %s com tamanho %d"), cd.comando, _tcslen(cd.comando) - 1);
	}
	//TRATA DO COMANDO VENDER AÇÕES
	else if (_tcsicmp(TEXT("sell"), cd.comando) == 0) {
		_tprintf(TEXT("\nRecebi do cliente o Comando: %s com tamanho %d"), cd.comando, _tcslen(cd.comando) - 1);
	}
	//TRATA DO COMANDO BALANCE
	else if (_tcsicmp(TEXT("balance"), cd.comando) == 0) {
		_tprintf(TEXT("\nRecebi do cliente o Comando: %s com tamanho %d"), cd.comando, _tcslen(cd.comando) - 1);
	}
	//TRATA DA FALHA DO COMANDO
	else {
		_tprintf(TEXT("\nComando do cliente: %s introduzido com tamanho %d, não existe!"), cd.comando, _tcslen(cd.comando) - 1);
	}
	
	return 0;
}
//-------
DWORD WINAPI verificaClientes() {
	
	FILE* fp;
	clienteData cd;
	BOOL resultado;
	DWORD nBytes;
	TCHAR string[100];
	HANDLE hSemClientes;
	HANDLE hPipe;

	//CRIAR O NAMEDPIPE BOLSA PARA INTERAÇÃO DOS CLIENTES
	hPipe = CreateNamedPipe(
		NOME,	// Nome do pipe
		PIPE_ACCESS_DUPLEX,			// acesso em modo de escrita e de leitura
		PIPE_TYPE_MESSAGE |			// message type pipe 
		PIPE_READMODE_MESSAGE |		// message-read mode 
		PIPE_WAIT,					// blocking mode 
		PIPE_UNLIMITED_INSTANCES,							// max. instances  
		sizeof(clienteData),					// output buffer size 
		sizeof(clienteData),					// input buffer size 
		0,							// client time-out 
		NULL);						// default security attribute 
	if (hPipe != INVALID_HANDLE_VALUE)
	{
		_tprintf(TEXT("[ERRO] Falhou ao criar CreateNamedPipe, %d.\n"), GetLastError());
		return -1;
	}
	_tprintf(TEXT("[ERRO] antes de esperar.\n"));
	resultado = ConnectNamedPipe(hPipe, INFINITE);
		if (resultado);
	_tprintf(TEXT("[ERRO] depois de esperar.\n"));
	/*
	while (1){
	//ReadFile(hPipe, &receivedInfo, sizeof(LoginInfo), &bytesRead, NULL)
	resultado = ReadFile(
		hPipe,
		&cd,
		sizeof(clienteData),
		&nBytes,
		NULL
	);
	if (resultado != ERROR_SUCCESS) {
		_tprintf(TEXT("[ERRO] Falhou ao ler o NamedPipe, %d.\n"), GetLastError());
		return -1;
	}
	_tprintf(TEXT("Recebi login: %zu, password: %zu"), cd.login, cd.password);

}*/















	/*
	hSemClientes = CreateSemaphoreA(
		NULL,
		5,
		5,
		TEXT("semaforoClientes")
		);
		if (hSemClientes == NULL) {
			_tprintf(TEXT("[ERRO] ao criar o semáforo: %d\n"),GetLastError());
			return -1;
		}
		*/
	














	fp = fopen(UTILIZADORES_REGISTADOS, "r"); //abertura do ficheiro Clientes.txt para leitura
	if (fp == NULL) //verifica se o ficheiro existe
	{
		_tprintf(TEXT("[ERRO] ao abrir o ficheiro! \n"));
		return -1;
	}



	while (fgetws(string, 100, fp)) {
		_tprintf(TEXT("%s \n"), string);
	}



	//CloseHandle(hSemClientes);
	CloseHandle(hPipe);
	fclose(fp);
}