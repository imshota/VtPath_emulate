/*  Pin v3.6 */
/*  VtPath emulater */
/*  実際の実行で、仮想パスを作り、VPテーブルにあるか調べる */

#include <stdio.h>
#include <map>
#include <iostream>
#include <string>
#include <stack>
#include <asm-generic/unistd.h>
#include "pin.H"

std::map<ADDRINT, std::string> funcnames;
std::stack<ADDRINT> preVStack, VStack;

unsigned long syscall_count = 0; //シスコールの数
unsigned long func_count = 0; //関数の数
/*****************************************************************************
 *                             Analysis functions                            *
 *****************************************************************************/

static void
push_func_return(ADDRINT ip)
{
  func_count++;
  printf("tets func : %ld\n", func_count);
  VStack.push(ip);
}

static void
push_syscall(ADDRINT ip)
{
  syscall_count++;
  printf("tets syscall : %ld\n", syscall_count);
  VStack.push(ip);
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
      //funcnames[RTN_Address(rtn)] = RTN_Name(rtn);
    }
  }
}

static void
instrument_fun(RTN rtn, void *v)
{
  RTN_Open(rtn);
  IMG imgfunc;
  //IMG imgins;
  imgfunc = IMG_FindByAddress(RTN_Address(rtn));
  if(!IMG_Valid(imgfunc) || !IMG_IsMainExecutable(imgfunc)) {
    RTN_Close(rtn);
    return;
  }
  funcnames[RTN_Address(rtn)] = RTN_Name(rtn);

  RTN_InsertCall(
    rtn, IPOINT_BEFORE, (AFUNPTR)push_func_return, 
    IARG_RETURN_IP,
    IARG_END
  );

  for(INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins)) {
    //imgins = IMG_FindByAddress(INS_Address(ins));
    //if(!IMG_Valid(imgins) || !IMG_IsMainExecutable(imgins)) continue;
    if(INS_IsSyscall(ins)) {
      INS_InsertCall(
        ins, IPOINT_BEFORE, (AFUNPTR)push_syscall, 
        IARG_INST_PTR,
        IARG_END
      );
    }
    createVtPath(preVStack, VStack);
    preVStack = VStack;
    VStack.pop();
  }
  VStack.pop();
  RTN_Close(rtn);
}

/*****************************************************************************
 *                               Other functions                             *
 *****************************************************************************/

/* VtPathを作る関数 */
static void
createVtPath(std::stack<ADDRINT> preVStack,std::stack<ADDRINT> VStack) {
  std::stack<ADDRINT> re_Stack, re_preVStack;
  ADDRINT item;
  /* それぞれスタックをひっくり返す */
  while (!VStack.empty())
  {
    item = VStack.top();
    re_VStack.push(item);
    VStack.pop();
  }
  while (!preVStack.empty())
  {
    item = preVStack.top();
    re_preVStack.push(item);
    preVStack.pop();
  }

  /* a_s != b_sになるまで調べる */
  while ( re_VStack.top() == re_preVStack.top() ||
          !re_VStack.empty() || !re_preVStack.empty() ) {
    re_VStack.pop();
    re_preVStack.pop();
  }

}

/* 結果（異常があるか）を出力。*/
static void
print_results(INT32 code, void *v)
{
  printf("\n******* FUNCTION CALLS *******\n");
  printf("function : %ld\n", func_count);
  /*for (std::map<ADDRINT, std::string>::iterator i = funcnames.begin(); i != funcnames.end(); ++i) {
        std::cout << i->first << " => " << i->second << std::endl;
  }*/

  printf("\n******* SYSCALLS *******\n");
  printf("syscall : %ld\n", syscall_count);

  /* これの実行で出てきたVPathが、トレーニングで培ったVP集合に入っているかチェック */

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
  RTN_AddInstrumentFunction(instrument_fun, NULL);

  PIN_AddFiniFunction(print_results, NULL);
  PIN_StartProgram();

  return 0;
}
