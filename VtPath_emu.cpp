/*  Pin v3.6 */
/*  VtPath emulater */
/*  実際の実行で、仮装パスを作り、RAテーブル、VPテーブルにあるか調べる */

#include <stdio.h>
#include <map>
#include <string>
#include <asm-generic/unistd.h>
#include "pin.H"

std::map<ADDRINT, unsigned long> > calls;
std::map<ADDRINT, std::string> funcnames;

unsigned long call_count    = 0; //呼び出し命令数
unsigned long syscall_count = 0; //シスコール数
/*****************************************************************************
 *                             Analysis functions                            *
 *****************************************************************************/

static void
log_syscall(ADDRINT ip)
{
  //syscalls[PIN_GetSyscallNumber(ctxt, std)]++;
  syscall_count++;
}

/*****************************************************************************
 *                         Instrumentation functions                         *
 *****************************************************************************/
static void
parse_funcsyms(IMG img, void *v)
{
  if(!IMG_Valid(img)) return;
  for(SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec)) {
    for(RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn)) {
      funcnames[RTN_Address(rtn)] = RTN_Name(rtn);
    }
  }
}

static void
instrument_fun(RTN fun, void *v)
{
  for(INS ins = RTN_InsHead(fun); INS_Valid(ins); ins = INS_Next(ins)) {
    if(INS_IsSyscall(ins)) {
      INS_InsertCall(
        ins, IPOINT_BEFORE, (AFUNPTR)count_call, 
        IARG_INST_PTR,
        IARG_END
      );
    }
  }
}

/*****************************************************************************
 *                               Other functions                             *
 *****************************************************************************/
static void
print_results(INT32 code, void *v)
{
  ADDRINT ip, target;
  unsigned long count;
  std::map<ADDRINT, std::map<ADDRINT, unsigned long> >::iterator i;
  std::map<ADDRINT, unsigned long>::iterator j;

  if(!calls.empty()) {
    printf("\n******* FUNCTION CALLS *******\n");
    printf("call : %ld", call_count);
    for(i = calls.begin(); i != calls.end(); i++) {
      target = i->first;
      for(j = i->second.begin(); j != i->second.end(); j++) {
        ip = j->first;
        count = j->second;
        printf("[%-30s] 0x%08jx <- 0x%08jx: %3lu (%0.2f%%)\n", 
               funcnames[target].c_str(), target, ip, count, (double)count/call_count*100.0);
      } 
    }
  }

  printf("\n******* SYSCALLS *******\n");
  printf("syscall : %ld", syscall_count);

}

static void
print_usage()
{
  std::string help = KNOB_BASE::StringKnobSummary();
  fprintf(stderr, "\nProfile call and jump targets\n");
  fprintf(stderr, "%s\n", help.c_str());
}

/*****************************************************************************
 *                               Main functions                             *
 *****************************************************************************/
int
main(int argc, char *argv[])
{
  /* Pinの初期化 */
  PIN_InitSymbols();
  if(PIN_Init(argc,argv)) {
    print_usage();
    return 1;
  }

  /* 計装ルーチンの登録 */
  IMG_AddInstrumentFunction(parse_funcsyms, NULL);
  RTN_AddInstrumentFunction	(instrument_fun, NULL);

  PIN_AddFiniFunction(print_results, NULL);
  PIN_StartProgram();

  return 0;
}
