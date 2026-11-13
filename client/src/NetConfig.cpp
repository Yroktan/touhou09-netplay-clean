#include "NetConfig.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace thnet {
static bool Error(char *target,unsigned size,const char *text) {
    if(size){strncpy(target,text,size-1);target[size-1]=0;}
    return false;
}
static char *Trim(char *text) {
    while(*text && isspace((unsigned char)*text))++text;
    char *end=text+strlen(text);
    while(end>text && isspace((unsigned char)end[-1]))--end;
    *end=0;return text;
}
static bool Copy(char *target,unsigned size,const char *text) {
    if(strlen(text)>=size)return false;
    strcpy(target,text);return true;
}
static bool ReadFile(char *host,char *port,char *error,unsigned errorSize) {
    char path[1024];
    const char *configured=getenv("TH09_ENV_FILE");
    if(configured) {
        if(!Copy(path,sizeof(path),configured))return Error(error,errorSize,"Configuration path is too long");
    } else {
#ifdef _WIN32
        DWORD length=GetModuleFileNameA(0,path,sizeof(path));
        if(!length || length>=sizeof(path)-1)
            return Error(error,errorSize,"Cannot locate .env beside the game EXE");
        char *slash=strrchr(path,'\\');
        if(!slash)return Error(error,errorSize,"Cannot locate .env beside the game EXE");
        strcpy(slash+1,".env");
#else
        strcpy(path,".env");
#endif
    }
    FILE *file=fopen(path,"rb");
    if(!file)return true;
    bool first=true,valid=true;
    char line[1024];
    while(fgets(line,sizeof(line),file)) {
        if(!strchr(line,'\n') && !feof(file)){valid=false;break;}
        char *key=line;
        if(first && strlen(key)>=3 && (unsigned char)key[0]==0xef && (unsigned char)key[1]==0xbb && (unsigned char)key[2]==0xbf)key+=3;
        first=false;key=Trim(key);
        if(!*key || *key=='#')continue;
        if(strncmp(key,"export ",7)==0)key=Trim(key+7);
        char *equals=strchr(key,'=');
        if(!equals){valid=false;break;}
        *equals=0;key=Trim(key);char *value=Trim(equals+1);
        if(!*key){valid=false;break;}
        unsigned length=(unsigned)strlen(value);
        if(length && (*value=='\'' || *value=='"')) {
            if(length<2 || value[length-1]!=*value){valid=false;break;}
            value[length-1]=0;++value;
        } else {
            char *comment=strchr(value,'#');if(comment)*comment=0;
            value=Trim(value);
        }
        if(strcmp(key,"TH09_RELAY_HOST")==0)valid=Copy(host,256,value);
        else if(strcmp(key,"TH09_RELAY_PORT")==0)valid=Copy(port,32,value);
        if(!valid)break;
    }
    if(ferror(file))valid=false;
    fclose(file);
    return valid || Error(error,errorSize,"Invalid .env configuration");
}
bool LoadRelayEndpoint(const char *host,unsigned short port,RelayEndpoint &endpoint,char *error,unsigned errorSize) {
    char configuredHost[256]={0},configuredPort[32]="10900";
    if(host && *host) {
        if(!Copy(configuredHost,sizeof(configuredHost),host))return Error(error,errorSize,"Relay hostname is too long");
        endpoint.port=port?port:10900;
    } else {
        if(!ReadFile(configuredHost,configuredPort,error,errorSize))return false;
        const char *value=getenv("TH09_RELAY_HOST");
        if(value && !Copy(configuredHost,sizeof(configuredHost),value))return Error(error,errorSize,"Relay hostname is too long");
        value=getenv("TH09_RELAY_PORT");
        if(value && !Copy(configuredPort,sizeof(configuredPort),value))return Error(error,errorSize,"Invalid TH09_RELAY_PORT");
        char *end=0;
        for(const char *digit=configuredPort;*digit;++digit)if(!isdigit((unsigned char)*digit))
            return Error(error,errorSize,"TH09_RELAY_PORT must be 1-65535");
        unsigned long number=strtoul(configuredPort,&end,10);
        if(!*configuredPort || *configuredPort=='-' || !end || *end || number<1 || number>65535)
            return Error(error,errorSize,"TH09_RELAY_PORT must be 1-65535");
        endpoint.port=port?port:(unsigned short)number;
    }
    char *trimmed=Trim(configuredHost);
    if(!*trimmed)return Error(error,errorSize,"Set TH09_RELAY_HOST in .env beside the EXE");
    for(char *p=trimmed;*p;++p)if(isspace((unsigned char)*p) || strchr("/\\:@#",*p))
        return Error(error,errorSize,"TH09_RELAY_HOST must be a hostname or IPv4");
    strcpy(endpoint.host,trimmed);
    if(errorSize)error[0]=0;
    return true;
}
}
