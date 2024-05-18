#include <windows.h>
#include <tchar.h>
#include <windowsx.h>
#include "Resource.h"
#include "..\utils.h"


LRESULT CALLBACK TrataEventos(HWND, UINT, WPARAM, LPARAM, PAINTSTRUCT);
BOOL CALLBACK DialogBoxAbout(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK DialogSettings(HWND, UINT, WPARAM, LPARAM);
DWORD WINAPI PaintThread(PVOID dados);

TCHAR szProgName[] = TEXT("Base"); // vai ser o nome do nosso programa, que vai aparecer no cantinho da janela

int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPTSTR lpCmdLine, int nCmdShow) {
	HWND hWnd;		// hWnd � o handler da janela, gerado mais abaixo por CreateWindow()
	MSG lpMsg;		// MSG � uma estrutura definida no Windows para as mensagens
	WNDCLASSEX wcApp;	// WNDCLASSEX � uma estrutura cujos membros servem para 
	// definir as caracter�sticas da classe da janela
	

	wcApp.cbSize = sizeof(WNDCLASSEX);      // Tamanho da estrutura WNDCLASSEX
	wcApp.hInstance = hInst;		         // Inst�ncia da janela actualmente exibida 
	// ("hInst" � par�metro de WinMain e vem 
		  // inicializada da�)
	wcApp.lpszClassName = szProgName;       // Nome da janela (neste caso = nome do programa)
	wcApp.lpfnWndProc = TrataEventos;       // Endere�o da fun��o de processamento da janela
	
	// ("TrataEventos" foi declarada no in�cio e
	// encontra-se mais abaixo)
	wcApp.style = CS_HREDRAW | CS_VREDRAW;  // Estilo da janela: Fazer o redraw se for
	// modificada horizontal ou verticalmente

	wcApp.hIcon = LoadIcon(NULL, IDI_APPLICATION);   // "hIcon" = handler do icon normal
	// "NULL" = Icon definido no Windows
	// "IDI_AP..." icone "aplicação"
	wcApp.hIconSm = LoadIcon(NULL, IDI_INFORMATION); // "hIconSm" = handler do icon pequeno
	// "NULL" = Icon definido no Windows
	// "IDI_INF..." Icon de informacao
	wcApp.hCursor = LoadCursor(NULL, IDC_ARROW);	// "hCursor" = handler do cursor (rato) 
	// "NULL" = Forma definida no Windows
	// "IDC_ARROW" Aspecto "seta" 
	wcApp.lpszMenuName = MAKEINTRESOURCE(IDC_BOARDGUI);	// Classe do menu que a janela pode ter
	// (NULL = não tem menu)
	wcApp.cbClsExtra = 0;				// Livre, para uso particular
	wcApp.cbWndExtra = sizeof(BoardGUIDados*);				// Livre, para uso particular
	wcApp.hbrBackground = CreateSolidBrush(RGB(22, 26, 37));
	
	if (!RegisterClassEx(&wcApp))
		return(0);

	
	hWnd = CreateWindow(
		szProgName,			// Nome da janela (programa) definido acima
		TEXT("BoardGui"),// Texto que figura na barra do t�tulo
		WS_OVERLAPPEDWINDOW,	// Estilo da janela (WS_OVERLAPPED= normal)
		CW_USEDEFAULT,		// Posi��o x pixels (default=� direita da ultima)
		CW_USEDEFAULT,		// Posi��o y pixels (default=abaixo da ultima)
		900,		// Largura da janela (em pixels)
		900,		// Altura da janela (em pixels)
		(HWND)HWND_DESKTOP,	// handle da janela pai (se se criar uma a partir de
		// outra) ou HWND_DESKTOP se a janela for a primeira, 
		// criada a partir do "desktop"
		(HMENU)NULL,			// handle do menu da janela (se tiver menu)
		(HINSTANCE)hInst,		// handle da instancia do programa actual ("hInst" � 
		// passado num dos par�metros de WinMain()
		0);
	
	
	ShowWindow(hWnd, nCmdShow);	// "hWnd"= handler da janela, devolvido por 
	// "CreateWindow"; "nCmdShow"= modo de exibi��o (p.e. 
	// normal/modal); � passado como par�metro de WinMain()
	UpdateWindow(hWnd);		// Refrescar a janela (Windows envia � janela uma 
	// mensagem para pintar, mostrar dados, (refrescar)� 



	while (GetMessage(&lpMsg, NULL, 0, 0) > 0) {
		TranslateMessage(&lpMsg);	// Pr�-processamento da mensagem (p.e. obter c�digo 
		// ASCII da tecla premida)
		DispatchMessage(&lpMsg);	// Enviar a mensagem traduzida de volta ao Windows, que
		// aguarda at� que a possa reenviar � fun��o de 
		// tratamento da janela, CALLBACK TrataEventos (abaixo)
	}


	return (int)lpMsg.wParam;	// Retorna sempre o par�metro wParam da estrutura lpMsg
}



DWORD WINAPI PaintThread(PVOID dados) {
	BoardGUIDados* bGUIdt = (BoardGUIDados*)dados;

	HANDLE hEvent, hMutex;
	DWORD aqui = 0;
	TCHAR buffer[256];
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
	while (bGUIdt->continua == 1)
	{

		//wsprintf(buffer, TEXT("\nCheguei aqui '%d'"), bGUIdt->continua);
		//OutputDebugString(buffer);
		WaitForSingleObject(hEvent, INFINITE);

		WaitForSingleObject(hMutex, INFINITE);

		ReleaseMutex(hMutex);
		InvalidateRect(bGUIdt->hWnd, NULL, TRUE);
		Sleep(200);
	}

	return 50;
}

BOOL CALLBACK DialogSettings(HWND hWndsettings, UINT messg, WPARAM wParam, LPARAM lParam) {
	BoardGUIDados* ptd = (BoardGUIDados*)GetWindowLongPtr(GetParent(hWndsettings), 0);
	int nEmpresas = 0, limInf = 0, limSup = 0;
	switch (messg) {
		case WM_INITDIALOG:
			SetDlgItemInt(hWndsettings, IDC_EDIT1, ptd->nEmpresas, FALSE);
			SetDlgItemInt(hWndsettings, IDC_EDIT2, ptd->limInf, FALSE);
			SetDlgItemInt(hWndsettings, IDC_EDIT3, ptd->limSup, FALSE);
			return TRUE;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
					ptd->nEmpresas = GetDlgItemInt(hWndsettings, IDC_EDIT1, NULL, FALSE);
					ptd->limInf = GetDlgItemInt(hWndsettings, IDC_EDIT2, NULL, FALSE);
					ptd->limSup = GetDlgItemInt(hWndsettings, IDC_EDIT3, NULL, FALSE);
					InvalidateRect(ptd->hWnd, NULL, TRUE);
					EndDialog(hWndsettings, IDOK);
					return TRUE;
				case IDCANCEL:
					EndDialog(hWndsettings, IDCANCEL);
					return TRUE;
			}
			break;
		case WM_CLOSE:
			EndDialog(hWndsettings, IDCANCEL);
			DestroyWindow(hWndsettings);
			return TRUE;
	}
	return FALSE;
}


LRESULT CALLBACK TrataEventos(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam, PAINTSTRUCT ps) {
	HDC hdc, hvalores;
	RECT rect, rect2;
	int xPos, yPos;
	BoardGUIDados* dados = (BoardGUIDados*)GetWindowLongPtr(hWnd, 0);


	//static TCHAR curChar = '?';
	
	float angulo = 45.0;
	float anguloemradianos = angulo * (3.14 / 180.0);
	HANDLE hMapFile;
	int resultado = 0;
	boardData* boardDt;
	DWORD Nempresas = 0;
	empresaData empresasBoard[MAX_EMPRESAS];
	HANDLE hThread = NULL;
	TCHAR string3[20];
	TCHAR string4[20];
	int escalaPos = 0;
	int escalaX = 50; // Posição horizontal da escala
	int escalaYStart = 300; // Posição vertical inicial da escala
	int escalaYEnd = 10; // Posição vertical final da escala
	int numLinhasEscala = 10; // Número de linhas na escala
	for (DWORD i = 0; i < MAX_EMPRESAS; i++)
	{
		_tcscpy(empresasBoard[i].nomeEmpresa, TEXT("-1"));
		empresasBoard[i].nAções = 0;
		empresasBoard[i].pAção = 0.0;
	}


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
	boardDt = (boardData*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(boardData));


	for (DWORD i = 0; i < MAX_EMPRESAS; i++)
	{
		_tcscpy(empresasBoard[i].nomeEmpresa, boardDt->empresas[i].nomeEmpresa);
		empresasBoard[i].nAções = boardDt->empresas[i].nAções;
		empresasBoard[i].pAção = boardDt->empresas[i].pAção;
	}

	switch (messg) {
	case WM_CREATE:
		dados = (BoardGUIDados*)malloc(sizeof(BoardGUIDados));
		SetWindowLongPtr(hWnd, 0, (LONG_PTR)dados);
		dados->hWnd = hWnd;
		dados->continua = 1;

		dados->nEmpresas = 10;
		dados->limInf = 0;
		dados->limSup = 500;

		hThread = CreateThread(NULL, 0, PaintThread, (LPVOID)dados, 0, NULL);
		
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);

		// Definir as dimensões e valores das barras
		RECT bars[30];
		//int values[5] = { 20, 50, 80, 30, 60 }; // Valores das barras
		int barWidth = 50; // Largura das barras
		int barSpacing = 20; // Espaçamento entre as barras
		int startX = 100; // Posição inicial das barras
		int startY = 300; // Posição vertical das barras
		int maxHeight = 300; // Altura máxima das barras
		int escalaY = dados->limSup/10;
		int valorEscY = dados->limSup;
		rect.left = 30;
		rect.top = 415;


		SetBkMode(hdc, TRANSPARENT);

		for (int i = 0; i < 30; i++)
		{
			if (_tcscmp(empresasBoard[i].nomeEmpresa, TEXT("-1")) != 0)
				Nempresas++;

		}
		

		SetTextColor(hdc, RGB(8,175, 13));
		// Desenhar as barras
		for (int i = 0; i < dados->nEmpresas; i++) {
			bars[i].left = startX + (barWidth + barSpacing) * i;
			bars[i].top = startY - (maxHeight * empresasBoard[i].pAção / dados->limSup);
			bars[i].right = bars[i].left + barWidth;
			bars[i].bottom = startY;

			FillRect(hdc, &bars[i], (HBRUSH)CreateSolidBrush(RGB(38, 166, 154)));

			if (_tcscmp(empresasBoard[i].nomeEmpresa, TEXT("-1")) != 0)
			{
				RECT textRect = bars[i];
				textRect.bottom = textRect.top;
				textRect.top -= 20;
				
				swprintf(string3, sizeof(string3) / sizeof(TCHAR), TEXT("%.2f€"), empresasBoard[i].pAção);
				DrawText(hdc, string3, -1, &textRect, DT_SINGLELINE | DT_NOCLIP | DT_CENTER);
			}
			if (_tcscmp(empresasBoard[i].nomeEmpresa, TEXT("-1")) != 0)
			{
				RECT textRect = bars[i];
				//textRect.bottom = textRect.top+ 60;
				textRect.bottom = 320;
				
				swprintf(string3, sizeof(string3) / sizeof(TCHAR), TEXT("%s"), empresasBoard[i].nomeEmpresa);
				
				DrawText(hdc, string3, -1, &textRect, DT_SINGLELINE | DT_NOCLIP | DT_BOTTOM| DT_WORDBREAK | DT_CENTER);
			}
		}



		SetTextColor(hdc, RGB(166, 169, 178));//RGB(166, 169, 71)
		for (int i = 0; i <= numLinhasEscala; i++) {
			int y = escalaYStart - (escalaYStart - escalaYEnd) * i / numLinhasEscala; // Ajuste para cima
			MoveToEx(hdc, escalaX, y, NULL);

			TCHAR labelText[20]; // Defina o tamanho conforme necessário
			swprintf(labelText, sizeof(labelText) / sizeof(TCHAR), TEXT("%.2f€"), (float)i * dados->limSup / numLinhasEscala);
			RECT textRect = { escalaX - 50, y - 10, escalaX, y };
			DrawText(hdc, labelText, -1, &textRect, DT_SINGLELINE | DT_NOCLIP); // Rótulos da escala
		}



		SetTextColor(hdc, RGB(38, 166, 154));
		//SetTextColor(hdc, RGB(rand() % 255, rand() % 255, rand() % 255));
		TCHAR string[200];
		TCHAR string2[200];
		if (_tcscmp(boardDt->ultmTransacao->EmpresaNome, TEXT("")) != 0) {
			DrawText(hdc, TEXT("Ultima Transação: "), -1, &rect, DT_SINGLELINE | DT_NOCLIP);
			rect.left = 150;
			DrawText(hdc, TEXT("Empresa: "), -1, &rect, DT_SINGLELINE | DT_NOCLIP);
			rect.left += (5*15)-1;
			DrawText(hdc, boardDt->ultmTransacao->EmpresaNome, -1, &rect, DT_SINGLELINE | DT_NOCLIP);

			rect.top = 430;
			rect.left = 150;
			_tcscpy(string, TEXT(""));
			DrawText(hdc, TEXT("Valor: "), -1, &rect, DT_SINGLELINE | DT_NOCLIP);
			rect.left += (5 * 15) - 1;
			swprintf(string, sizeof(string) / sizeof(TCHAR), TEXT("%lu"), boardDt->ultmTransacao->nAcoes);
			DrawText(hdc, string, -1, &rect, DT_SINGLELINE | DT_NOCLIP);

			rect.top = 445;
			rect.left = 150;
			_tcscpy(string2, TEXT(""));
			DrawText(hdc, TEXT("Custo: "), -1, &rect, DT_SINGLELINE | DT_NOCLIP);
			rect.left += (5 * 15) - 1;
			swprintf(string2, sizeof(string2) / sizeof(TCHAR), TEXT("%.2f€"), boardDt->ultmTransacao->pAcao);
			DrawText(hdc, string2, -1, &rect, DT_SINGLELINE | DT_NOCLIP);

		}


		EndPaint(hWnd, &ps);
		break;
	case WM_COMMAND:
		switch (wParam)
		{
			case IDM_EXIT:
				if (MessageBox(hWnd, TEXT("Tem a certeza que quer sair?"),
					TEXT("Confirmação"), MB_ICONQUESTION | MB_YESNO) == IDYES)
				{
					DestroyWindow(hWnd);
				}
				break;
			case IDM_ABOUT:
				DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, DialogBoxAbout);
				break;
			case ID_FILE_SETTINGS:
				DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_SETTINGS), hWnd, DialogSettings);
				break;
			default:
				break;
		}
		break;
	case WM_CLOSE:

		if (MessageBox(hWnd, TEXT("Tem a certeza que quer sair?"),
			TEXT("Confirmação"), MB_ICONQUESTION | MB_YESNO) == IDYES)
		{
			DestroyWindow(hWnd);
		}
		dados->continua = 0;
		break;
	case WM_DESTROY:	// Destruir a janela e terminar o programa 
		// "PostQuitMessage(Exit Status)"	
		UnmapViewOfFile(hMapFile);
		CloseHandle(hThread);	
		free(dados);
		PostQuitMessage(0);
		break;
	default:
		// Neste exemplo, para qualquer outra mensagem (p.e. "minimizar","maximizar","restaurar")
		// n�o � efectuado nenhum processamento, apenas se segue o "default" do Windows
		return(DefWindowProc(hWnd, messg, wParam, lParam));
		break;  // break tecnicamente desnecess�rio por causa do return
	}

	

	return 0;
}


BOOL CALLBACK DialogBoxAbout(HWND hWndialog, UINT messg, WPARAM wParam, LPARAM lParam) {

	switch (messg) {
		case WM_INITDIALOG:
			return TRUE;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
					EndDialog(hWndialog, IDOK);
					return TRUE;
				//case IDCANCEL: 
				//	EndDialog(hWnd, IDCANCEL);
				//	return TRUE;
			}
		case WM_CLOSE:
			EndDialog(hWndialog, IDOK); // janela criada com DialogBox()
			DestroyWindow(hWndialog); // janela criado com CreateDialog()
			return TRUE;
		}
	return FALSE;
}

