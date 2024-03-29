// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <QtNetwork/QNetworkAccessManager>
#include <detours/detours.h>
#include <QtNetwork/QHttpMultiPart>


HANDLE hThread;
HINSTANCE mod;

typedef QNetworkReply* (__thiscall *tPost1)(QNetworkAccessManager*,const QNetworkRequest&, QIODevice*);
typedef QNetworkReply* (__thiscall *tPost2)(QNetworkAccessManager*,const QNetworkRequest&, QByteArray&);
typedef QNetworkReply* (__thiscall *tPost3)(QNetworkAccessManager*,const QNetworkRequest&, QHttpMultiPart*);

typedef QNetworkReply* (__thiscall *tGet)(QNetworkAccessManager*,const QNetworkRequest&);


tPost1 fPost1 = 0;
tPost2 fPost2 = 0;
tPost3 fPost3 = 0;
tGet fGet = 0;

BOOL WINAPI __HandlerRoutine(DWORD dwCtrlType);

bool HookFunction(void** toHook, void* interception, const char* name) {
    if (toHook == 0) {
        printf("ERROR - Failed to hook function %s\n", name);
        return false;
    }
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(toHook, interception);

    LONG lError = DetourTransactionCommit();
    if (lError != NO_ERROR) {
        printf("ERROR - Failed to hook function %s\n", name);
        return false;
    }

    return true;
}

bool UnhookFunction(void** toHook, void* interception) {
    if (toHook == 0) {
        return false;
    }
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach(toHook, interception);

    LONG lError = DetourTransactionCommit();
    if (lError != NO_ERROR) {
        return false;
    }

    return true;
}


void openConsole()
{
    if (AllocConsole()) {
        freopen("CONOUT$", "wt", stdout);
        SetConsoleTitle(L"QtHooker");
        SetConsoleCtrlHandler(__HandlerRoutine, 1);
        system("cls");
    }
}


DWORD __declspec(naked) getReturnValue(){
    __asm {
        MOV EAX, [EBP + 4]
        RET
    }
}




void printRequest(const QNetworkRequest& request, const QByteArray& data,DWORD callingFunction,const char* type = "POST") {
    printf("[%s]\n", type);
    printf("CallingFunction: %#x\n", callingFunction);
    printf("URL: %s\n",request.url().toString().toUtf8().constData());

    QList<QByteArray> lst = request.rawHeaderList();

    if (lst.count()>0) {
        printf("Headers\n");
        for (int i = 0; i < lst.count(); i++) {
            auto header = lst[i];
            printf("%s : %s\n", header.constData(),request.rawHeader(header).constData());
        }
        printf("}\n");
    }

    if (data.size() > 0) {
        printf("  Content:%s\n", data.constData());
    }
}



QNetworkReply* __fastcall postHook1(QNetworkAccessManager* man, void* EDX, const QNetworkRequest& request, QIODevice* data) {
    DWORD returnAddr = getReturnValue();
    QByteArray byteData = data->readAll();
    printRequest(request,byteData,returnAddr);
    return fPost1(man, request, data);
}


QNetworkReply* __fastcall postHook2(QNetworkAccessManager* man, void* EDX, const QNetworkRequest& request, QByteArray& data) {
    DWORD returnAddr = getReturnValue();

    printRequest(request, data, returnAddr);
    return fPost2(man, request, data);
}

QNetworkReply* __fastcall postHook3(QNetworkAccessManager* man, void* EDX, const QNetworkRequest& request, QHttpMultiPart* multiPart) {
    DWORD returnAddr = getReturnValue();

    QByteArray byteData = multiPart->boundary();
    printRequest(request, byteData, returnAddr);
    return fPost3(man, request, multiPart);
}

QNetworkReply* __fastcall getHook(QNetworkAccessManager* man, void* EDX, const QNetworkRequest& request) {
    DWORD returnAddr = getReturnValue();

    QByteArray data;
    printRequest(request, data, returnAddr, "GET");
    return fGet(man, request);
}



void Leave() {
    UnhookFunction((void**)&fPost1, postHook1);
    UnhookFunction((void**)&fPost2, postHook2);
    UnhookFunction((void**)&fPost3, postHook3);
    UnhookFunction((void**)&fGet, getHook);
    FreeConsole();
    //FreeLibrary(mod);
}

BOOL WINAPI __HandlerRoutine(DWORD dwCtrlType) {
    switch (dwCtrlType) {
    case CTRL_C_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        Leave();
    }
    return true;
}



void mainIntercept() {
    openConsole();
    HMODULE qtNetwork = GetModuleHandle(L"Qt5Network.dll");

    if (!qtNetwork) {
        printf("Error - Cannot find Qt5Network.dll\n");
        Leave();
        return;
    }


    //These values probably change from implementation to implementation
    fPost1 = (tPost1)GetProcAddress(qtNetwork, MAKEINTRESOURCEA(869)); //public: class QNetworkReply * __thiscall QNetworkAccessManager::post(class QNetworkRequest const &,class QIODevice *)
    fPost2 = (tPost2)GetProcAddress(qtNetwork, MAKEINTRESOURCEA(867)); //public: class QNetworkReply * __thiscall QNetworkAccessManager::post(class QNetworkRequest const &,class QByteArray const &)
    fPost3 = (tPost3)GetProcAddress(qtNetwork, MAKEINTRESOURCEA(868)); //public: class QNetworkReply * __thiscall QNetworkAccessManager::post(class QNetworkRequest const &,class QHttpMultiPart *)

    fGet = (tGet)GetProcAddress(qtNetwork, MAKEINTRESOURCEA(605)); //public: class QNetworkReply* __thiscall QNetworkAccessManager::get(class QNetworkRequest const&)

    if (HookFunction((void**)&fPost1, postHook1, "Post1") && HookFunction((void**)&fPost2, postHook2, "Post2") && HookFunction((void**)&fPost3, postHook3, "Post3") && HookFunction((void**)&fGet, getHook, "Get")) {
        printf("Functions Hooked with success!\n\n");
    }
}





BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        mod = hModule;
        mainIntercept();
        //hThread = CreateThread(NULL, NULL, &mainIntercept, &mod, NULL, NULL);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        //Leave();
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

