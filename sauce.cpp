#undef UNICODE
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
HMODULE DLLhandle;
typedef void *(WINAPI* A2Malloc)(SIZE_T);
typedef void (WINAPI* A2Free)(void*);
A2Malloc MemAlloc = *(A2Malloc*)0xDBF2A0;
A2Free MemFree = *(A2Free*)0xDBF2A4;
class GameState;
class GameNamespace;
class GameEvaluator;
class ArmAString;
typedef int(__thiscall *TExecuteScript)(GameState* thisPtr, char* script, void* flags, GameNamespace* ns);
TExecuteScript ExecuteScript = (TExecuteScript)0x9D8DA3;

class GameState
{
public:
	char _0x0000[328];
	GameEvaluator* GEvaluator; //0x0148 
	char _0x014C[17];
	BYTE ShowScriptErrors; //0x015D
	char _0x015A[716];
	void Execute(char* script, BYTE* EvalContext, GameNamespace* ns) {
		ExecuteScript(this, script, EvalContext, ns);
	}

};//Size=0x0426

class GameEvaluator
{
public:
	char _0x0000[16];
	__int32 _error; //0x0010 - the Error number can use the Enum for clarification 
	ArmAString* ErrorText; //0x0014 - the 'Error'
	char _0x0018[12];
	ArmAString* errorContent; //0x0024 - the script that has the error in it
	__int32 _pos; //0x0028 - the position the error is in the text
	char _0x002C[20];

	enum EvalError
	{ 
		EvalOK,
		EvalGen, // generic error
		EvalExpo, // exponent out of range or invalid
		EvalNum, // invalid number
		EvalVar, // undefined variable
		EvalBadVar, // undefined variable
		EvalDivZero, // zero divisor
		EvalTg90, // tg 90
		EvalOpenB, // missing (
		EvalCloseB, // missing )
		EvalOpenBrackets, // missing [
		EvalCloseBrackets, // missing ]
		EvalOpenBraces, // missing {
		EvalCloseBraces, // missing }
		EvalEqu, // missing =
		EvalSemicolon, // missing ;
		EvalQuote, // missing "
		EvalSingleQuote, // missing '
		EvalOper, // unknown operator
		EvalLineLong, // line too long
		EvalType, // type mismatch
		EvalNamespace, // no name space
		EvalDim, // array dimension
		EvalUnexpectedCloseB, // unexpected )
		EvalAssertFailed, //assertion failed error
		EvalHaltCalled,  //forced error using halt function
		EvalForeignError, //user error from foreign modules
		EvalScopeNameExists, //unable to define scopeName twice
		EvalScopeNotFound, //scope has not been found
		EvalInvalidTryBlock,  //invalid usage of try block
		EvalUnhandledException,  //Exception has been thrown, but none of catch handles this exceptions
		EvalStackOverflow, //stack overflow during processing BeginContext
		EvalHandled, // error occurred, but was already handled (used when the error occurs in the inner scope)
	};
};//Size=0x0040

class ArmAString
{
public:
	__int32 References; //0x0000 
	__int32 StrLen; //0x0004 
	char Str[1]; //0x0008 

};//Size=0x000C

class GameNamespace
{
public:
	char _0x0000[16];
};

class World
{
public:
	char _0x0000[1536];
	GameNamespace* missionNamespace;
};

World* world = *(World**)0xDA8208;
BYTE* ExecuteFlags = (BYTE*)0xD841F8;
GameState* Gstate = (GameState*)0xDBF6C0;

/*
void ErrorCheckScript(char* script) //Only Will get errors IF ITS NOT RUN BY THE GAME ITSELF
{
	Gstate->Execute(script, ExecuteFlags, world->missionNamespace); //Let the game parse and run it and we will just grab the error
	GameEvaluator *_e = (GameEvaluator*)Gstate->GEvaluator; //Pointer to the script evaluator
	if (_e != nullptr)
	{
		ArmAString *content = (ArmAString*)_e->errorContent;//Get the script that has the error
		if (content != nullptr)
		{
			ArmAString *errortxt = (ArmAString*)_e->ErrorText;
			if (errortxt != nullptr)
			{
				int ErrorPosition = _e->_pos; //The REAL error is +1 ahead it leaves a space for the |#| indentifier
				char *initialString = content->Str;
				char *ErrorString = errortxt->Str;
				char string[1024] = {}; //Errors can be big
				sprintf(string, "%s", initialString); //Move the read string into a writable character array
				append(string, "|#|", ErrorPosition);
				int endOfScript = strlen(string);
				printf("Error Type: %s\n%s\nError: %s\n", string,ErrorString); //TODO: Make WinAPI and allow for file recignition: Constant reading\comparing
				bool notStartScript = false;
				bool notEndScript = false;
				int start = 0;
				if (ErrorPosition > 41)
				{
					notStartScript = true;
					start = ErrorPosition - 40;
				}
				if (endOfScript > ErrorPosition + 40)
				{
					notEndScript = true;
					endOfScript = ErrorPosition + 40;
				}

			}
		}
	}

}
*/


HWND hEdit, ExecuteButton, ExecuteAndDebug, ErrorLog, CurLine;

void DebugLog(char* text)
{
	int len = SendMessage(ErrorLog, WM_GETTEXTLENGTH, 0, 0);
	SendMessage(ErrorLog, EM_SETSEL, (WPARAM)len, (LPARAM)len);
	SendMessage(ErrorLog, EM_REPLACESEL, 0, (LPARAM)text);
}

char* StripComments(char* input)
{
	return "";
}

void ExecuteScriptAPI(bool isDebug)
{
	char *Text;
	int len = SendMessage(hEdit, WM_GETTEXTLENGTH, 0, 0);
	Text = (char*)MemAlloc(len + 1); //Because we dont know how much text there is + null terminator
	SendMessage(hEdit, WM_GETTEXT, (WPARAM)len + 1, (LPARAM)Text);
	int lines = SendMessage(hEdit, EM_GETLINECOUNT, 0, 0);
	char buf[256];
	sprintf(buf, "Compiling %i characters, %i lines...\n", len,lines);
	//Text = StripComments(Text);
	//DebugLog("Stripping Comments...\n");
	Gstate->Execute((char*)Text, ExecuteFlags, world->missionNamespace);
	DebugLog(buf);
	if (isDebug)
	{
		GameEvaluator *_e = (GameEvaluator*)Gstate->GEvaluator;
		ArmAString *errorText = (ArmAString*)_e->ErrorText;
		int position = _e->_pos;
		if (errorText != nullptr)
		{
			ArmAString *content = (ArmAString*)_e->errorContent;
			char buffer[256];
			SendMessage(hEdit, EM_SETSEL, (WPARAM)position + 1, (LPARAM)position + 1);
			SetFocus(hEdit);
			int line = SendMessage(hEdit, EM_LINEFROMCHAR, -1, 0);
			sprintf(buffer, "Error: %s on Line %i\n", errorText->Str, line+1);
			DebugLog(buffer);
		}
		else
		{
			DebugLog("No Errors Detected\n");
		}
	}
	else
	{
		DebugLog("Script Compiled!\n");
	}
	MemFree(Text);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_CREATE:
		{
			HFONT hFont = CreateFont(20, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Arial");
			HFONT hFontSmall = CreateFont(15, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Lucida Console");
			
			hEdit = CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT", "",
				WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
				0, 0, 600, 400, hwnd, (HMENU)155, GetModuleHandle(NULL), NULL);
			
			SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0));
			SendMessage(hEdit, EM_SETLIMITTEXT, (WPARAM)-1, 0);

			ErrorLog = CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT", "",
				WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
				0, 405, 600, 150, hwnd, (HMENU)156, GetModuleHandle(NULL), NULL);
			SendMessage(ErrorLog, WM_SETFONT, (WPARAM)hFontSmall, MAKELPARAM(FALSE, 0));
			EnableWindow(GetDlgItem(hwnd, 165), FALSE);

			ExecuteButton = CreateWindowW(L"BUTTON", L"Execute", WS_CHILD | WS_VISIBLE, 650, 15, 100, 24, hwnd, (HMENU)12, GetModuleHandle(NULL), NULL);
			ExecuteAndDebug = CreateWindowW(L"BUTTON", L"Execute+Debug", WS_CHILD | WS_VISIBLE, 645, 45, 110, 24, hwnd, (HMENU)13, GetModuleHandle(NULL), NULL);
			CurLine = CreateWindowW(L"Static", L"Line ", WS_CHILD | WS_VISIBLE, 600, 385, 100, 125,hwnd,(HMENU)1135, GetModuleHandle(NULL), NULL);
		}
		break;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			case 12:
				ExecuteScriptAPI(false);
				break;
			case 13:
				ExecuteScriptAPI(true);
				break;
			}
		break;
		case WM_CLOSE:
			DestroyWindow(hwnd);
			FreeLibraryAndExitThread(DLLhandle,0);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
		break;
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI OkeyDokey()
{
	MSG  msg;
	HWND hwnd;
	WNDCLASSEX wc;
	const char w_className[] = "ScriptDebugger";

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = NULL;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = w_className;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);


	RegisterClassEx(&wc);
	hwnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		w_className,
		"Script Debugger",
		WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
		NULL, NULL, NULL, NULL);

	ShowWindow(hwnd, true);
	UpdateWindow(hwnd);

	while (GetMessage(&msg, NULL, 0, 0)) {
		int line = SendMessage(hEdit, EM_LINEFROMCHAR, -1, 0);
		char buf[64];
		sprintf(buf, "Line %i", line + 1);
		SendMessage(CurLine, WM_SETTEXT, 0, (LPARAM)&buf);
		if (msg.message == WM_KEYDOWN && GetFocus() == GetDlgItem(hwnd, 156))
			DispatchMessage(&msg);
		else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_KEYDOWN && msg.wParam == 'A' && GetKeyState(VK_CONTROL) < 0)
			{
				HWND isFocused = GetFocus();
				char classname[6];
				GetClassName(isFocused, classname, 6);
				if (isFocused && !_stricmp(classname, "edit"))
					SendMessage(isFocused, EM_SETSEL, 0, -1);
			}
		}
	}
	return msg.wParam;
}
BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
	)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		//AllocConsole();
		//freopen("CONOUT$", "wt", stdout);
		//freopen("CONIN$", "rb", stdin);
		DLLhandle = hModule;
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)OkeyDokey, NULL, NULL, NULL);
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
