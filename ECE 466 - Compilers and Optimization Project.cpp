/*------------------------------------------------
Project 2: Common Subexpression Elimination
Steven Yan and Matthew Gribbins
ECE466
4/2/2021
About: LLVM Pass that does Common subexpression elim
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
#include "llvm/PassRegistry.h"

//#include "dominance.h"

using namespace llvm;


// CSE Functions
static void CommonSubexpressionElimination(Module *);
static bool isDead(Instruction &I);
static bool RunDeadCodeElimination(Module *M);
static void MakeDT(Module *M);
static void recursiveCall(LLVMBasicBlockRef bb, Instruction &I);
static bool opcodeSkip(Instruction &I);
static void DeadCodeElimination(Module *M);
static void SimplifyInstructionPass(Module *M);
static void CSE(Module *M);

// dominance.cpp functions
static void UpdateDominators(Function *F);
static LLVMBasicBlockRef LLVMFirstDomChild(LLVMBasicBlockRef BB);
static LLVMBasicBlockRef LLVMNextDomChild(LLVMBasicBlockRef BB, LLVMBasicBlockRef Child);


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
                cl::desc("Perform memory to register promotion before CSE."),
                cl::init(false));

static cl::opt<bool>
        NoCSE("no-cse",
              cl::desc("Do not perform CSE Optimization."),
              cl::init(false));

static cl::opt<bool>
        Verbose("verbose",
                    cl::desc("Verbose stats."),
                    cl::init(false));

static cl::opt<bool>
        NoCheck("no",
                cl::desc("Do not check for valid IR."),
                cl::init(false));

static llvm::Statistic WorkList = {"", "WorkList", "Added to work list"};

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
    if (Mem2Reg)
    {
        legacy::PassManager Passes;
        Passes.add(createPromoteMemoryToRegisterPass());
        Passes.run(*M.get());
    }

    if (!NoCSE) {
        CommonSubexpressionElimination(M.get());
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

static llvm::Statistic CSEDead = {"", "CSEDead", "CSE eliminated dead instructions"};
static llvm::Statistic CSEElim = {"", "CSEElim", "CSE eliminated redundant instructions"};
static llvm::Statistic CSESimplify = {"", "CSESimplify", "CSE simplified instructions"};



// Common Subexpression Elimination controller function
static void CommonSubexpressionElimination(Module *M) {
    DeadCodeElimination(M);
    SimplifyInstructionPass(M);
    CSE(M);
}

// Pass to check for dead code
// Eliminates any useless instructions
// Adds instructions to worklist to be deleted
static void DeadCodeElimination(Module *M) {
    std::set<Instruction*> worklist;

    for (auto f = M->begin(); f != M->end(); f++) {
        // loop over functions
        for (auto bb = f->begin(); bb != f->end(); bb++) {
            // loop over basic blocks
            for (auto i = bb->begin(); i != bb->end(); i++) {
                //loop over instructions
                if (isDead(*i)) {
                    // if Dead, insert to worklist to be deleted
                    worklist.insert(&*i);
                    WorkList++;
                }
            }
        }
    }

    while(worklist.size()>0)
    {
        // Get the first item
        Instruction *j = *(worklist.begin());
        // Erase it from worklist
        worklist.erase(j);
        // Erase from basic block
        j->eraseFromParent();
        CSEDead++;
    }
}

// Dead checking function
// Used to determine if a given instruction is able to be deleted
bool isDead(Instruction &I) {
    int opcode = I.getOpcode();
    switch(opcode){
        case Instruction::Add:
        case Instruction::FNeg:
        case Instruction::FAdd:
        case Instruction::Sub:
        case Instruction::FSub:
        case Instruction::Mul:
        case Instruction::FMul:
        case Instruction::UDiv:
        case Instruction::SDiv:
        case Instruction::FDiv:
        case Instruction::URem:
        case Instruction::SRem:
        case Instruction::FRem:
        case Instruction::Shl:
        case Instruction::LShr:
        case Instruction::AShr:
        case Instruction::And:
        case Instruction::Or:
        case Instruction::Xor:
        case Instruction::Alloca:
        case Instruction::GetElementPtr:
        case Instruction::Trunc:
        case Instruction::ZExt:
        case Instruction::SExt:
        case Instruction::FPToUI:
        case Instruction::FPToSI:
        case Instruction::UIToFP:
        case Instruction::SIToFP:
        case Instruction::FPTrunc:
        case Instruction::FPExt:
        case Instruction::PtrToInt:
        case Instruction::IntToPtr:
        case Instruction::AddrSpaceCast:
        case Instruction::ICmp:
        case Instruction::FCmp:
        case Instruction::PHI:
        case Instruction::Select:
        case Instruction::ExtractElement:
        case Instruction::InsertElement:
        case Instruction::ShuffleVector:
        case Instruction::ExtractValue:
        case Instruction::InsertValue:
            if ( I.use_begin() == I.use_end() )
            {
                return true; // dead, but this is not enough
            }
            break;
        case Instruction::Load:
        {
            LoadInst *li = dyn_cast<LoadInst>(&I);
            if (li && li->isVolatile())
                return false;
            if (I.use_begin() == I.use_end())
                return true;
            break;

        }
        default:
            // any other opcode fails
            return false;
    }
    return false;
}

// Simplify Instruction Pass
// Used to simplify instructions using a variety of methods
// These include constant folding and other simplifications
// Replaces a function with its simplification, before deleting
static void SimplifyInstructionPass(Module *M) {
    std::set<Instruction*> worklist;

    for (auto f = M->begin(); f != M->end(); f++) {
        // loop over functions
        for (auto bb = f->begin(); bb != f->end(); bb++) {
            // loop over basic blocks
            for (auto i = bb->begin(); i != bb->end(); i++) {
                //loop over instructions
                auto j = i;
                Value *tmp = SimplifyInstruction(&*i, M->getDataLayout());
                if (tmp != nullptr) {
                    CSESimplify++;
                    (*j).replaceAllUsesWith(tmp);
                    worklist.insert(&*i);
                }
            }
        }
    }
    while(worklist.size()>0)
    {
        // Get the first item
        Instruction *k = *(worklist.begin());
        // Erase it from worklist
        worklist.erase(k);
        // Erase from basic block
        k->eraseFromParent();
    }
}

// Main Common Subexpression Elimination Function
// Loops over all instructions and determines dominance
// Eliminates instructions recursively in order to find edge cases
// Erases unnecessary instructions after replacing them with useful instructions
static void CSE(Module *M) {
    for (auto f = M->begin(); f != M->end(); f++) {
        // loop over functions
        for (auto bb = f->begin(); bb != f->end(); bb++) {
            // loop over basic blocks
            for (auto i = bb->begin(); i != bb->end(); i++) {
                //loop over instructions
                for (auto j = bb->begin(); j != bb->end(); ) {
                    if (i == j) {
                        j++;
                    }
                    else if((*j).isIdenticalTo(&*i) && opcodeSkip(*i) ) { //
                        j->replaceAllUsesWith(&*i);
                        j = j->eraseFromParent();
                        CSEElim++;
                    }
                    else {
                        j++;
                    }
                }
                recursiveCall(wrap(&*bb), *i);
            }
        }
    }
}

// Recursive Function for use with CSE
// Loops over basic blocks and the dominator tree to determine dominance
// Eliminates instructions and delves past the first dominator tree
// Utilizes dominance.cpp functions
void recursiveCall(LLVMBasicBlockRef bb, Instruction &I) {
    auto child = unwrap(LLVMFirstDomChild(bb));
    while(child != NULL) {
        for(auto j = child->begin(); j != child->end(); ) {
            // if instruction matches
            if((*j).isIdenticalTo(&I) && opcodeSkip(I) ) { //
                j->replaceAllUsesWith(&I);
                j = j->eraseFromParent();
                CSEElim++;
            }
            else { j++; }
        }
        recursiveCall(wrap(child), I);
        child = unwrap(LLVMNextDomChild(bb, wrap(&*child)));
    }

}

// Checks for skippable instructions that cannot be simplified
// These include loads, stores, and allocas
// If we encounter one of these types, we cannot consider it in CSE
bool opcodeSkip(Instruction &I) {
    auto opcode = I.getOpcode();
    switch (opcode) {
        case Instruction::Load: return false;
        case Instruction::Store: return false;
        case Instruction::Alloca: return false;
        case Instruction::Call: return false;
        case Instruction::FCmp: return false;
        case Instruction::Ret: return false;
        case Instruction::Br: return false;
        case Instruction::Switch: return false;
        case Instruction::Invoke: return false;
        case Instruction::IndirectBr: return false;
        case Instruction::CallBr: return false;
        case Instruction::Resume: return false;
        case Instruction::CatchSwitch: return false;
        case Instruction::CatchRet: return false;
        case Instruction::CleanupRet: return false;
        case Instruction::Unreachable: return false;
        case Instruction::VAArg: return false;
    }
    if (I.isTerminator() || I.mayHaveSideEffects()) {
        return false;
    }
    else {
        return true;
    }
}

// Dominance.cpp file that updates dominator tree based on the input function
void UpdateDominators(Function *F)
{
    if (Current != F)
    {
        Current = F;

        if (DT==NULL)
        {
            DT = new DominatorTreeBase<BasicBlock,false>();
            PDT = new DominatorTreeBase<BasicBlock,true>();
            if (LI==NULL)
                LI = new LoopInfoBase<BasicBlock,Loop>();
        }

        DT->recalculate(*F);
        PDT->recalculate(*F);

        LI->analyze(*DT);
    }
}

// Dominance.cpp file that returns the first dominator child
LLVMBasicBlockRef LLVMFirstDomChild(LLVMBasicBlockRef BB)
{
    UpdateDominators(unwrap(BB)->getParent());
    DomTreeNodeBase<BasicBlock> *Node = DT->getNode(unwrap(BB));

    if(Node==NULL)
        return NULL;

    DomTreeNodeBase<BasicBlock>::iterator it = Node->begin();
    if (it!=Node->end())
        return wrap((*it)->getBlock());
    return NULL;
}

// Dominance.cpp file that returns the next dominator child based on the current child
LLVMBasicBlockRef LLVMNextDomChild(LLVMBasicBlockRef BB, LLVMBasicBlockRef Child)
{
    UpdateDominators(unwrap(BB)->getParent());
    DomTreeNodeBase<BasicBlock> *Node = DT->getNode(unwrap(BB));
    DomTreeNodeBase<BasicBlock>::iterator it,end;

    bool next=false;
    for(it=Node->begin(),end=Node->end(); it!=end; it++)
        if (next)
            return wrap((*it)->getBlock());
        else if (*it==DT->getNode(unwrap(Child)))
            next=true;

    return NULL;
}