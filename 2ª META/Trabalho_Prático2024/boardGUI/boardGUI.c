#include <windows.h>
#include <tchar.h>
#include <windowsx.h>
#include "Resource.h"
#include "..\utils.h"


LRESULT CALLBACK TrataEventos(HWND, UINT, WPARAM, LPARAM);

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
	wcApp.lpszMenuName = MAKEINTRESOURCE(IDC_BOARDGUI);			// Classe do menu que a janela pode ter
	// (NULL = não tem menu)
	wcApp.cbClsExtra = 0;				// Livre, para uso particular
	wcApp.cbWndExtra = 0;				// Livre, para uso particular
	wcApp.hbrBackground = CreateSolidBrush(RGB(22, 26, 37));
	
	if (!RegisterClassEx(&wcApp))
		return(0);

	
	hWnd = CreateWindow(
		szProgName,			// Nome da janela (programa) definido acima
		TEXT("BoardGui"),// Texto que figura na barra do t�tulo
		WS_OVERLAPPEDWINDOW,	// Estilo da janela (WS_OVERLAPPED= normal)
		CW_USEDEFAULT,		// Posi��o x pixels (default=� direita da ultima)
		CW_USEDEFAULT,		// Posi��o y pixels (default=abaixo da ultima)
		800,		// Largura da janela (em pixels)
		600,		// Altura da janela (em pixels)
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
LRESULT CALLBACK TrataEventos(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam, PAINTSTRUCT ps) {
	HDC hdc;
	RECT rect;
	int xPos, yPos;
	static TCHAR curChar = '?';
	float angulo = 45.0;
	float anguloemradianos = angulo * (3.14 / 180.0);
	HANDLE hEvent, hMutex, hMapFile;
	boardData* boardDt;
	DWORD Nempresas = 0;
	empresaData empresasBoard[MAX_EMPRESAS];

	for (DWORD i = 0; i < MAX_EMPRESAS; i++)
	{
		_tcscpy(empresasBoard[i].nomeEmpresa, TEXT("-1"));
		empresasBoard[i].nAções = 0;
		empresasBoard[i].pAção = 0.0;
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

	for (DWORD i = 0; i < MAX_EMPRESAS; i++)
	{
		_tcscpy(empresasBoard[i].nomeEmpresa, boardDt->empresas[i].nomeEmpresa);
		empresasBoard[i].nAções = boardDt->empresas[i].nAções;
		empresasBoard[i].pAção = boardDt->empresas[i].pAção;
	}

	switch (messg) {
	case WM_PAINT:
		

		HDC hdc = BeginPaint(hWnd, &ps);
		
		// Definir as dimensões e valores das barras
		RECT bars[30];
		//int values[5] = { 20, 50, 80, 30, 60 }; // Valores das barras
		int barWidth = 40; // Largura das barras
		int barSpacing = 20; // Espaçamento entre as barras
		int startX = 50; // Posição inicial das barras
		int startY = 400; // Posição vertical das barras
		int maxHeight = 300; // Altura máxima das barras
		
		rect.left = 30;
		rect.top = 415;

		SetTextColor(hdc, RGB(255, 255, 255));
		SetBkMode(hdc, TRANSPARENT);

		for (int i = 0; i < 30; i++)
		{
			if (_tcscmp(empresasBoard[i].nomeEmpresa, TEXT("-1")) != 0)
				Nempresas++;

		}
	





		// Desenhar as barras
		for (int i = 0; i < Nempresas; i++) {
			bars[i].left = startX + (barWidth + barSpacing) * i;
			bars[i].top = startY - (maxHeight * empresasBoard[i].pAção / 500);
			bars[i].right = bars[i].left + barWidth;
			bars[i].bottom = startY;

			FillRect(hdc, &bars[i], (HBRUSH)CreateSolidBrush(RGB(38, 166, 154)));

			if (_tcscmp(empresasBoard[i].nomeEmpresa, TEXT("-1")) != 0)
			{
				RECT textRect = bars[i];
				textRect.bottom = textRect.top;
				textRect.top -= 20;
				SetTextColor(hdc, RGB(rand() % 255, rand() % 255, rand() % 255));

				DrawText(hdc, empresasBoard[i].nomeEmpresa, -1, &textRect, DT_SINGLELINE | DT_NOCLIP | DT_CENTER);
				//DrawText(hdc, empresasBoard[i].nomeEmpresa, -1, &rect, DT_SINGLELINE | DT_NOCLIP);
				//DrawText(hdc, empresasBoard[i].nomeEmpresa, -1, &rect, DT_SINGLELINE | DT_NOCLIP);
				//rect.top = startY + empresasBoard[i].pAção;
				//rect.left += 20;
			}
		}








		EndPaint(hWnd, &ps);
		break;
	case WM_LBUTTONDOWN:

		break;
	
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDM_EXIT:
			if (MessageBox(hWnd, TEXT("Tem a certeza que quer sair?"),
				TEXT("Confirmação"), MB_ICONQUESTION | MB_YESNO) == IDYES)
			{
				DestroyWindow(hWnd);
			}
			break;
		case IDM_ABOUT:
			MessageBox(hWnd, TEXT("Olá?"),
				TEXT("Confirmação"), MB_ICONQUESTION | MB_YESNO) == IDYES;
		default:
			break;
		}

	case WM_CLOSE:

		if (MessageBox(hWnd, TEXT("Tem a certeza que quer sair?"),
			TEXT("Confirmação"), MB_ICONQUESTION | MB_YESNO) == IDYES)
		{
			DestroyWindow(hWnd);
		}
		break;
	case WM_DESTROY:	// Destruir a janela e terminar o programa 
		// "PostQuitMessage(Exit Status)"		
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
