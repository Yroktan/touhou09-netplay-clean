                                                                        
                                                                     
                                                                         
                                                                    
                  
  
                                                                       
                                                                       
                                                                     
                                    
  
                                     
                                             
                                                               
                                                                     
                                                                        
                                            
                                        
                                                                  
                           
  
                                                                   
                                                                        

#ifndef TRACE_H
#define TRACE_H

#include <windows.h>

#ifdef __cplusplus
extern "C"
{
#endif

    enum TraceKind
    {
                                            
        TRACE_KIND_CALC = 1,
                                                          
        TRACE_KIND_STAGE_END = 2,
                              
        TRACE_KIND_STAGE_START = 3,
                                                                       
        TRACE_KIND_REPLAY_END = 4,
                                                                           
        TRACE_KIND_TITLE = 5,
                                         
        TRACE_KIND_LIMIT = 6,
                                                                  
                                          
        TRACE_KIND_CRASH = 7,
    };

                   

                                                                       
                              
    DWORD TraceEnvNumber(const char *name, DWORD otherwise);
                                                                          
    BOOL TraceEnvString(const char *name, char *buf, DWORD size);
                                                                   
                                                               
    BOOL TraceReadAddrs(const char **names, DWORD *addrs, int count);
                                                                      
                        
    DWORD TraceNextNumber(const char **s);

                                   

                                                                  
    BOOL TraceWriteCode(void *at, const void *bytes, DWORD size);
                                                                        
                                                                     
                                                
    int TracePatchImmediates(DWORD start, DWORD size, DWORD from, DWORD to);
                                                                         
                                               
    int TracePatchCalls(DWORD start, DWORD size, DWORD from, DWORD to);
                                                                          
                                        
    void *TracePatchImport(const char *dll, const char *name, void *to);

                    

                                           
    BOOL TraceOpen(const char *path);
                                                                       
    void TraceInfo(const char *fmt, ...);
                                                                     
                                                                  
    void TraceWriteHeader(const char *names, DWORD count);
                           
    void TraceWrite(const void *record, DWORD size);
                                                                
    void TraceFlush(void);
                                                                          
    void TraceExit(UINT code);

                          

                                                                   
                                                                   
                                                                        
                                                                     
                                            
    BOOL TraceInstallClock(void);
                                                                         
                                                                       
                                                    
    BOOL TraceInstallRenderer(void);
                                                                  
                                                                         
                                                
    void TraceOnCrash(void (*crashed)(DWORD code, DWORD address));

#ifdef __cplusplus
}
#endif

#endif
