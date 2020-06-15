#!/usr/bin/env python2
## -*- coding: utf-8 -*-

# 全てのパスを通る入力を計算する。

import sys

# tritonとpintoolをインポート
import triton
import pintool

Triton = pintool.getTritonContext()

def symbolize_inputs(tid):
    rdi = pintool.getCurrentRegisterValue(Triton.registers.rdi) # argc
    rsi = pintool.getCurrentRegisterValue(Triton.registers.rsi) # argv

    # for each string in argv
    while rdi > 1:
        addr = pintool.getCurrentMemoryValue(rsi + ((rdi-1)*triton.CPUSIZE.QWORD), triton.CPUSIZE.QWORD)
        # symbolize the current argument string (including the terminating NULL)
        c = None
        s = ''
        while c != 0:
            c = pintool.getCurrentMemoryValue(addr)
            s += chr(c)
            Triton.setConcreteMemoryValue(addr, c)
            Triton.convertMemoryToSymbolicVariable(triton.MemoryAccess(addr, triton.CPUSIZE.BYTE)).setComment('argv[%d][%d]' % (rdi-1, len(s)-1))
            addr += 1
        rdi -= 1
        print 'Symbolized argument %d: %s' % (rdi, s)

def find_input(insn, op):
    regId   = Triton.getSymbolicRegisterId(op)
    regExpr = Triton.unrollAst(Triton.getAstFromId(regId))
    ast = Triton.getAstContext()
    pcl = Triton.getPathConstraints()
    for pc in pcl:
        if pc.isMultipleBranches():
            b1 = pc.getBranchConstraints()[0]['constraint']
            b2 = pc.getBranchConstraints()[1]['constraint']

    #print('Constraint branch 1: %s' % (b1))
    #print('Constraint branch 2: %s' % (b2))

    exploitExpr = b1
    #exploitExpr = ast.equal(regExpr, ast.bv(target, triton.CPUSIZE.QWORD_BIT))
    for k, v in Triton.getSymbolicVariables().iteritems():
        if 'argv' in v.getComment():
            # Argument characters must be printable
            argExpr = Triton.getAstFromId(k)
            argExpr = ast.land([
                          ast.bvuge(argExpr, ast.bv(32,  triton.CPUSIZE.BYTE_BIT)),
                          ast.bvule(argExpr, ast.bv(126, triton.CPUSIZE.BYTE_BIT))
                      ])
            exploitExpr = ast.land([exploitExpr, argExpr])

    print 'Getting model for %s to %s' % (insn, exploitExpr)
    model = Triton.getModel(exploitExpr)
    for k, v in model.iteritems():
        print '%s (%s)' % (v, Triton.getSymbolicVariableFromId(k).getComment())    

def hook_cmp(insn):
    if insn.isControlFlow() and insn.isBranch():
        for op in insn.getOperands():
            if op.getType() == triton.OPERAND.REG:
                address = hex(insn.getAddress())
                print '分岐アドレス', address
                find_input(insn, op)

def main():
    # Tritonアーキテクチャを設定
    # x86-64にハードコード
    Triton.setArchitecture(triton.ARCH.X86_64)
    # ALIGNED_MEMORY最適化を有効
    Triton.enableMode(triton.MODE.ALIGNED_MEMORY, True)

    # 'main'という関数から計装開始
    # シンボルが使えない時はpintool.startAnalysisFromAddressでアドレスを指定
    pintool.startAnalysisFromSymbol('main')

    # mainでユーザからの入力をシンボル化
    pintool.insertCall(symbolize_inputs, pintool.INSERT_POINT.ROUTINE_ENTRY, 'main')
    # 全てののパスを通る入力を見つける。
    pintool.insertCall(hook_cmp, pintool.INSERT_POINT.BEFORE)

    pintool.runProgram()

if __name__ == '__main__':
    main()