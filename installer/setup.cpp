#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wincrypt.h>
#include <shlobj.h>
#include <shellapi.h>
#include <string>
#include <vector>
#include <stdio.h>
#include <wctype.h>
#include "NetConfig.h"
#include "payload.h"

static HWND windowHandle,folderEdit,hostEdit,portEdit,statusLabel,launchBox,installButton;
static std::wstring lastError;

static std::wstring Join(const std::wstring &folder,const wchar_t *name) {
    return folder+(folder.empty() || folder[folder.size()-1]==L'\\'?L"":L"\\")+name;
}
static std::wstring Wide(const std::string &text) {
    int count=MultiByteToWideChar(CP_UTF8,0,text.c_str(),-1,0,0);
    if(!count)return L"Invalid text";
    std::vector<wchar_t> buffer(count);
    MultiByteToWideChar(CP_UTF8,0,text.c_str(),-1,&buffer[0],count);
    return &buffer[0];
}
static std::string Utf8(const std::wstring &text) {
    int count=WideCharToMultiByte(CP_UTF8,0,text.c_str(),-1,0,0,0,0);
    if(!count)return "";
    std::vector<char> buffer(count);
    WideCharToMultiByte(CP_UTF8,0,text.c_str(),-1,&buffer[0],count,0,0);
    return &buffer[0];
}
static std::wstring Trim(std::wstring text) {
    while(!text.empty() && iswspace(text[0]))text.erase(0,1);
    while(!text.empty() && iswspace(text[text.size()-1]))text.erase(text.size()-1);
    return text;
}
static bool Exists(const std::wstring &path) {
    return GetFileAttributesW(path.c_str())!=INVALID_FILE_ATTRIBUTES;
}
static bool Fail(const wchar_t *message) {
    lastError=message;return false;
}
static bool HashMatches(const std::wstring &path,const wchar_t *expected) {
    HANDLE file=CreateFileW(path.c_str(),GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,0);
    if(file==INVALID_HANDLE_VALUE)return false;
    HCRYPTPROV provider=0;HCRYPTHASH hash=0;
    bool ok=CryptAcquireContextW(&provider,0,0,PROV_RSA_AES,CRYPT_VERIFYCONTEXT)!=0;
    if(ok)ok=CryptCreateHash(provider,CALG_SHA1,0,0,&hash)!=0;
    BYTE buffer[65536];DWORD read=0;
    while(ok) {
        if(!ReadFile(file,buffer,sizeof(buffer),&read,0)){ok=false;break;}
        if(!read)break;
        if(!CryptHashData(hash,buffer,read,0)){ok=false;break;}
    }
    BYTE digest[20];DWORD size=sizeof(digest);
    if(ok)ok=CryptGetHashParam(hash,HP_HASHVAL,digest,&size,0)!=0;
    wchar_t hex[41];
    if(ok)for(int i=0;i<20;++i)swprintf(hex+i*2,3,L"%02x",(unsigned)digest[i]);
    if(hash)CryptDestroyHash(hash);
    if(provider)CryptReleaseContext(provider,0);
    CloseHandle(file);
    return ok && _wcsicmp(hex,expected)==0;
}
static std::wstring UniqueSuffix() {
    SYSTEMTIME time;GetSystemTime(&time);
    wchar_t value[80];
    swprintf(value,80,L"%04u%02u%02u-%02u%02u%02u-%lu-%lu",time.wYear,time.wMonth,time.wDay,time.wHour,time.wMinute,time.wSecond,GetCurrentProcessId(),GetTickCount());
    return value;
}
static bool AtomicWrite(const std::wstring &path,const unsigned char *data,DWORD size,std::wstring &backup) {
    std::wstring suffix=UniqueSuffix();
    std::wstring temporary=path+L".install-"+suffix;
    HANDLE file=CreateFileW(temporary.c_str(),GENERIC_WRITE,0,0,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,0);
    if(file==INVALID_HANDLE_VALUE)return Fail(L"Cannot write to the game folder. Check permissions or run Setup as administrator.");
    DWORD written=0;
    bool ok=WriteFile(file,data,size,&written,0)!=0 && written==size && FlushFileBuffers(file)!=0;
    CloseHandle(file);
    if(ok) {
        if(Exists(path)) {
            backup=path+L".backup-"+suffix;
            ok=ReplaceFileW(path.c_str(),temporary.c_str(),backup.c_str(),REPLACEFILE_WRITE_THROUGH,0,0)!=0;
        } else {
            backup.clear();
            ok=MoveFileExW(temporary.c_str(),path.c_str(),MOVEFILE_WRITE_THROUGH)!=0;
        }
    }
    if(!ok) {
        DeleteFileW(temporary.c_str());
        return Fail(L"Cannot replace a mod file. Close the game and check folder permissions.");
    }
    return true;
}
static void Restore(const std::wstring &path,const std::wstring &backup) {
    if(backup.empty())DeleteFileW(path.c_str());
    else MoveFileExW(backup.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH);
}
static bool Install(const std::wstring &directory,const std::wstring &hostText,const std::wstring &portText) {
    std::wstring folder=Trim(directory),host=Trim(hostText),port=Trim(portText);
    DWORD attributes=GetFileAttributesW(folder.c_str());
    if(folder.empty() || attributes==INVALID_FILE_ATTRIBUTES || !(attributes&FILE_ATTRIBUTE_DIRECTORY))
        return Fail(L"Select an existing Touhou 09 1.50a game folder.");
    if(host.empty())return Fail(L"Enter the relay hostname provided by the server owner.");
    wchar_t ascii[256];
    if(!IdnToAscii(0,host.c_str(),-1,ascii,256))return Fail(L"Invalid relay hostname.");
    host=ascii;
    if(port.empty())return Fail(L"Enter a port from 1 to 65535.");
    unsigned number=0;
    for(unsigned i=0;i<port.size();++i) {
        if(port[i]<L'0' || port[i]>L'9')return Fail(L"Port must be a number from 1 to 65535.");
        number=number*10+(port[i]-L'0');
        if(number>65535)return Fail(L"Port must be a number from 1 to 65535.");
    }
    if(!number)return Fail(L"Port must be a number from 1 to 65535.");
    thnet::RelayEndpoint endpoint;char error[128];
    std::string encodedHost=Utf8(host);
    if(!thnet::LoadRelayEndpoint(encodedHost.c_str(),(unsigned short)number,endpoint,error,sizeof(error))) {
        lastError=Wide(error);return false;
    }
    if(!HashMatches(Join(folder,L"th09.dat"),L"fc810cbd80bd05ba9b24aedfeefad03250128e83") ||
       !HashMatches(Join(folder,L"thbgm.dat"),L"b0d04325d7ab1d9c0cd4ed94b008058c08926277"))
        return Fail(L"Compatible Touhou 09 1.50a game data is required (th09.dat and thbgm.dat). Steam is not required.");
    std::wstring exe=Join(folder,L"th09-netplay.exe"),env=Join(folder,L".env"),license=Join(folder,L"th09-netplay-LICENSE.txt");
    std::wstring exeBackup,envBackup,licenseBackup;
    char portBuffer[16];sprintf(portBuffer,"%u",number);
    std::string configuration="TH09_RELAY_HOST="+encodedHost+"\r\nTH09_RELAY_PORT="+portBuffer+"\r\n";
    if(!AtomicWrite(exe,kClientExe,(DWORD)sizeof(kClientExe),exeBackup))return false;
    if(!AtomicWrite(env,(const unsigned char*)configuration.data(),(DWORD)configuration.size(),envBackup)) {
        Restore(exe,exeBackup);return false;
    }
    if(!AtomicWrite(license,kLicense,(DWORD)sizeof(kLicense),licenseBackup)) {
        Restore(env,envBackup);Restore(exe,exeBackup);return false;
    }
    return true;
}
static std::wstring Text(HWND edit) {
    int length=GetWindowTextLengthW(edit);
    std::vector<wchar_t> value(length+1);
    GetWindowTextW(edit,&value[0],length+1);
    return &value[0];
}
static HWND Control(HWND parent,const wchar_t *kind,const wchar_t *text,DWORD style,int x,int y,int width,int height,int id) {
    HWND handle=CreateWindowExW(kind==std::wstring(L"EDIT")?WS_EX_CLIENTEDGE:0,kind,text,WS_CHILD|WS_VISIBLE|style,x,y,width,height,parent,(HMENU)(INT_PTR)id,GetModuleHandleW(0),0);
    SendMessageW(handle,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),TRUE);
    return handle;
}
static void Launch(const std::wstring &folder) {
    std::wstring exe=Join(Trim(folder),L"th09-netplay.exe");
    ShellExecuteW(windowHandle,L"open",exe.c_str(),0,Trim(folder).c_str(),SW_SHOWNORMAL);
}
static LRESULT CALLBACK WindowProc(HWND window,UINT message,WPARAM wParam,LPARAM lParam) {
    if(message==WM_CREATE) {
        Control(window,L"STATIC",L"Touhou 09 Netplay",0,22,18,530,24,0);
        Control(window,L"STATIC",L"Install the client into your Touhou 09 1.50a folder. Game data is not included.",0,22,48,540,30,0);
        Control(window,L"STATIC",L"Game folder",0,22,88,440,20,0);
        folderEdit=Control(window,L"EDIT",L"",ES_AUTOHSCROLL|WS_TABSTOP,22,110,440,25,101);
        Control(window,L"BUTTON",L"Browse...",WS_TABSTOP,472,110,90,25,102);
        Control(window,L"STATIC",L"Relay hostname or IPv4 (from the server owner)",0,22,152,470,20,0);
        hostEdit=Control(window,L"EDIT",L"",ES_AUTOHSCROLL|WS_TABSTOP,22,175,400,25,103);
        Control(window,L"STATIC",L"UDP port",0,440,152,120,20,0);
        portEdit=Control(window,L"EDIT",L"10900",ES_NUMBER|ES_AUTOHSCROLL|WS_TABSTOP,440,175,122,25,104);
        launchBox=Control(window,L"BUTTON",L"Launch the game after installation",BS_AUTOCHECKBOX|WS_TABSTOP,22,222,430,24,105);
        SendMessageW(launchBox,BM_SETCHECK,BST_CHECKED,0);
        statusLabel=Control(window,L"STATIC",L"Steam and compatible non-Steam 1.50a copies are supported.",0,22,263,540,38,106);
        installButton=Control(window,L"BUTTON",L"Install",BS_DEFPUSHBUTTON|WS_TABSTOP,352,325,100,30,107);
        Control(window,L"BUTTON",L"Cancel",WS_TABSTOP,462,325,100,30,108);
        SetFocus(folderEdit);
        return 0;
    }
    if(message==WM_COMMAND) {
        if(LOWORD(wParam)==102) {
            BROWSEINFOW browse={0};browse.hwndOwner=window;
            browse.lpszTitle=L"Select the folder containing th09.dat and thbgm.dat";
            browse.ulFlags=BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE;
            LPITEMIDLIST selected=SHBrowseForFolderW(&browse);
            if(selected) {
                wchar_t path[MAX_PATH];
                if(SHGetPathFromIDListW(selected,path))SetWindowTextW(folderEdit,path);
                CoTaskMemFree(selected);
            }
            return 0;
        }
        if(LOWORD(wParam)==108){DestroyWindow(window);return 0;}
        if(LOWORD(wParam)==107) {
            EnableWindow(installButton,FALSE);SetWindowTextW(statusLabel,L"Checking game files and installing...");UpdateWindow(window);
            bool result=Install(Text(folderEdit),Text(hostEdit),Text(portEdit));
            EnableWindow(installButton,TRUE);
            if(result) {
                SetWindowTextW(statusLabel,L"Installed. Connection settings are saved in .env beside the game EXE.");
                MessageBoxW(window,L"Touhou 09 Netplay is installed. Open NETPLAY to create or join a room.",L"Installation complete",MB_OK|MB_ICONINFORMATION);
                if(SendMessageW(launchBox,BM_GETCHECK,0,0)==BST_CHECKED)Launch(Text(folderEdit));
                DestroyWindow(window);
            } else {
                SetWindowTextW(statusLabel,L"Installation was not completed.");
                MessageBoxW(window,lastError.c_str(),L"Installation failed",MB_OK|MB_ICONERROR);
            }
            return 0;
        }
    }
    if(message==WM_DESTROY){PostQuitMessage(0);return 0;}
    return DefWindowProcW(window,message,wParam,lParam);
}
int WINAPI WinMain(HINSTANCE instance,HINSTANCE,LPSTR,int show) {
    int count=0;LPWSTR *arguments=CommandLineToArgvW(GetCommandLineW(),&count);
    if(count>1) {
        bool silent=false,launch=false;std::wstring directory,host,port=L"10900";
        for(int i=1;i<count;++i) {
            std::wstring argument=arguments[i];
            if(argument==L"--silent" || argument==L"/S")silent=true;
            else if(argument==L"--launch")launch=true;
            else if((argument==L"--directory" || argument==L"--host" || argument==L"--port") && i+1<count) {
                std::wstring value=arguments[++i];
                if(argument==L"--directory")directory=value;
                else if(argument==L"--host")host=value;
                else port=value;
            } else {LocalFree(arguments);return 2;}
        }
        LocalFree(arguments);
        if(!silent)return 2;
        if(!Install(directory,host,port))return 1;
        if(launch)Launch(directory);
        return 0;
    }
    if(arguments)LocalFree(arguments);
    CoInitializeEx(0,COINIT_APARTMENTTHREADED);
    WNDCLASSW type={0};type.lpfnWndProc=WindowProc;type.hInstance=instance;
    type.hCursor=LoadCursorW(0,IDC_ARROW);type.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);type.lpszClassName=L"TH09NetplaySetup";
    if(!RegisterClassW(&type))return 1;
    RECT area;SystemParametersInfoW(SPI_GETWORKAREA,0,&area,0);
    windowHandle=CreateWindowExW(WS_EX_DLGMODALFRAME,type.lpszClassName,L"Touhou 09 Netplay Setup",WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,
        (area.left+area.right-600)/2,(area.top+area.bottom-410)/2,600,410,0,0,instance,0);
    if(!windowHandle)return 1;
    ShowWindow(windowHandle,show);UpdateWindow(windowHandle);
    MSG message;
    while(GetMessageW(&message,0,0,0)>0)if(!IsDialogMessageW(windowHandle,&message)){TranslateMessage(&message);DispatchMessageW(&message);}
    CoUninitialize();
    return (int)message.wParam;
}
