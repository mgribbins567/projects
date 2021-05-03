/*------------------------------------------------
Project 2: Loop Invariant Code Motion
Steven Yan and Matthew Gribbins
ECE466
4/29/2021
About: Implementation of LICM using LoopAPI
------------------------------------------------*/

#include <fstream>
#include <memory>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "llvm-c/Core.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/Bitcode/BitcodeReader.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/LinkAllPasses.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Analysis/InstructionSimplify.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Analysis/LoopInfo.h"
#include <iostream>

using namespace llvm;

static void LoopInvariantCodeMotion(Module *);
static bool opcodeSkip(Instruction &I);

static void summarize(Module *M);
static void print_csv_file(std::string outputfile);

Function *Current=NULL;
DominatorTreeBase<BasicBlock,false> *DT=NULL;
DominatorTreeBase<BasicBlock,true> *PDT=NULL;

LoopInfoBase<BasicBlock,Loop> *LI=NULL;

static cl::opt<std::string>
        InputFilename(cl::Positional, cl::desc("<input bitcode>"), cl::Required, cl::init("-"));

static cl::opt<std::string>
        OutputFilename(cl::Positional, cl::desc("<output bitcode>"), cl::Required, cl::init("out.bc"));

static cl::opt<bool>
        Mem2Reg("mem2reg",
                cl::desc("Perform memory to register promotion before LICM."),
                cl::init(false));

static cl::opt<bool>
        CSE("cse",
            cl::desc("Perform CSE before LICM."),
            cl::init(false));

static cl::opt<bool>
        NoLICM("no-licm",
               cl::desc("Do not perform LICM optimization."),
               cl::init(false));

static cl::opt<bool>
        Verbose("verbose",
                cl::desc("Verbose stats."),
                cl::init(false));

static cl::opt<bool>
        NoCheck("no",
                cl::desc("Do not check for valid IR."),
                cl::init(false));

int main(int argc, char **argv) {
    // Parse command line arguments
    cl::ParseCommandLineOptions(argc, argv, "llvm system compiler\n");

    // Handle creating output files and shutting down properly
    llvm_shutdown_obj Y;  // Call llvm_shutdown() on exit.
    LLVMContext Context;

    // LLVM idiom for constructing output file.
    std::unique_ptr<ToolOutputFile> Out;
    std::string ErrorInfo;
    std::error_code EC;
    Out.reset(new ToolOutputFile(OutputFilename.c_str(), EC,
                                 sys::fs::F_None));

    EnableStatistics();

    // Read in module
    SMDiagnostic Err;
    std::unique_ptr<Module> M;
    M = parseIRFile(InputFilename, Err, Context);

    // If errors, fail
    if (M.get() == 0)
    {
        Err.print(argv[0], errs());
        return 1;
    }

    // If requested, do some early optimizations
    if (Mem2Reg || CSE)
    {
        legacy::PassManager Passes;
        if (Mem2Reg)
            Passes.add(createPromoteMemoryToRegisterPass());
        if (CSE)
            Passes.add(createEarlyCSEPass());
        Passes.run(*M.get());
    }

    if (!NoLICM) {
        LoopInvariantCodeMotion(M.get());
    }

    // Collect statistics on Module
    summarize(M.get());
    print_csv_file(OutputFilename);

    if (Verbose)
        PrintStatistics(errs());

    // Verify integrity of Module, do this by default
    if (!NoCheck)
    {
        legacy::PassManager Passes;
        Passes.add(createVerifierPass());
        Passes.run(*M.get());
    }

    // Write final bitcode
    WriteBitcodeToFile(*M.get(), Out->os());
    Out->keep();

    return 0;
}

static llvm::Statistic nFunctions = {"", "Functions", "number of functions"};
static llvm::Statistic nInstructions = {"", "Instructions", "number of instructions"};
static llvm::Statistic nLoads = {"", "Loads", "number of loads"};
static llvm::Statistic nStores = {"", "Stores", "number of stores"};

static void summarize(Module *M) {
    for (auto i = M->begin(); i != M->end(); i++) {
        if (i->begin() != i->end()) {
            nFunctions++;
        }

        for (auto j = i->begin(); j != i->end(); j++) {
            for (auto k = j->begin(); k != j->end(); k++) {
                Instruction &I = *k;
                nInstructions++;
                if (isa<LoadInst>(&I)) {
                    nLoads++;
                } else if (isa<StoreInst>(&I)) {
                    nStores++;
                }
            }
        }
    }
}

static void print_csv_file(std::string outputfile)
{
    std::ofstream stats(outputfile + ".stats");
    auto a = GetStatistics();
    for (auto p : a) {
        stats << p.first.str() << "," << p.second << std::endl;
    }
    stats.close();
}

static llvm::Statistic LICMBasic = {"", "LICMBasic", "basic loop invariant instructions"};
//static llvm::Statistic LICMLoadHoist = {"", "LICMLoadHoist", "loop invariant load instructions"};
static llvm::Statistic LICMNoPreheader = {"", "LICMNoPreheader", "absence of preheader prevents optimization"};
//static llvm::Statistic LICMBadStore = {"", "LICMBadStore", "aliasing store prevents optimization"};
//static llvm::Statistic LICMBadCall = {"", "LICMBadCall", "call instruction prevents optimization"};
static llvm::Statistic NumLoops = {"", "NumLoops", "number of loops analyzed"};
static llvm::Statistic NumLoopsNoLoad = {"", "NumLoopsNoLoad", "number of loops without a load"};
static llvm::Statistic NumLoopsNoStore = {"", "NumLoopsNoStore", "number of loops without a store"};
static llvm::Statistic NumLoopsNoStoreWithLoad = {"", "NumLoopsNoStoreWithLoad", "number of loops without a store, but with a load"};
static llvm::Statistic NumLoopsWithCall = {"", "NumLoopsWithCall", "number of loops with a call"};



static void LoopInvariantCodeMotion(Module *M) {

    //Flags for loop statistics
    bool loadCheck = true;
    bool storeCheck = true;
    bool callCheck = true;
    // Loop through each function of the module
    for (auto F = M->begin(); F != M->end(); F++) {
        LoopInfoBase <BasicBlock, Loop> *LI = new LoopInfoBase<BasicBlock, Loop>();
        auto DT = new DominatorTreeBase<BasicBlock, false>();
        // If Function has a size > 0, recalculate the dominator tree
        if (F->size() > 0) { //recalculate for each function with atleast 1 BB
            DT->recalculate(*F);
            LI->analyze(*DT);
            // Loop through each loop in the dominator tree
            for (auto li: *LI) {
                NumLoops++;  //increment counter
                auto PH = li->getLoopPreheader();  //get Loop-preheader
                if (PH == NULL) {
                    LICMNoPreheader++;
                    return;
                }
                // Loop through each basic block in the loop
                for (auto bb: li->blocks()) {
                    // Loop through each instruction in the basic block
                    for (auto i = bb->begin(); i != bb->end();) {
                        bool changed = false; //boolean variable
                        auto opcode = (*i).getOpcode();  //pull the opcode of the current instruction
                        auto j = i;  //create temporary iterator
                        j++;  //increment the temp to the instruction after the current
                        // If instruction is invariant in loop, instruction is not a Load, and instruction is not a Store
                        if(opcodeSkip(*i)) {

                            if (li->makeLoopInvariant(&*i, changed)){ //makeLoopInvariant function call returns a boolean
                                i = j;  //make iterator point to the previous stored
                                LICMBasic++;  //increment counter
                            }
                            else {
                                i++;  //if not loop Invariant increment iterator anyway
                            }
                        }
                        else {
                            i++;  //if a load or store skip over
                        }



                        // If Instruction is a Load
                        if (opcode == Instruction::Load) {
                            loadCheck = false;
                        }
                        // If Instruction is a Store
                        if (opcode == Instruction::Store) {
                            storeCheck = false;
                        }
                        // If Instruction is a Call
                        if (opcode == Instruction::Call) {
                            callCheck = false;
                        }

                    }
                }
                // If there is a Load, Store, or Call in the loop, these are checked and then reset
                // Checked for every loop after the basic blocks are examined
                if (loadCheck) {
                    NumLoopsNoLoad++;
                }
                if (storeCheck) {
                    NumLoopsNoStore++;
                }
                if (!storeCheck && loadCheck) {
                    NumLoopsNoStoreWithLoad++;
                }
                if (!callCheck) {
                    NumLoopsWithCall++;
                }

                //Reset the flags at the end of the loop
                storeCheck = true;
                loadCheck = true;
                callCheck = true;
            }
        }
    }
}

static bool opcodeSkip(Instruction &I) {
    auto opcode = I.getOpcode();
    switch(opcode) {
        case Instruction::Load:  //skip loads
            return false;
        case Instruction::Store: //skip stores
            return false;
            /* case Instruction::Alloca:
                return false;
            case Instruction::GetElementPtr:
                return false;
            case Instruction::BitCast:
            return false; */
        default:
            return true;
    }
}