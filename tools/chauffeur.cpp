//
// Copyright (c) 2014-2015 Pantazis Deligiannis (p.deligiannis@imperial.ac.uk)
// This file is distributed under the MIT License. See LICENSE for details.
//

#include "chauffeur/Chauffeur.h"
#include "chauffeur/ParseDriverConsumer.h"
#include "clang/Driver/Options.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Option/OptTable.h"
#include <iostream>
#include <sstream>
#include <vector>


using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;

static std::unique_ptr<llvm::opt::OptTable> Options(createDriverOptTable());

static cl::opt<bool> ASTDump("ast-dump",
    cl::desc(Options->getOptionHelpText(options::OPT_ast_dump)));
static cl::opt<bool> ASTList("ast-list",
    cl::desc(Options->getOptionHelpText(options::OPT_ast_list)));
static cl::opt<bool> ASTPrint("ast-print",
    cl::desc(Options->getOptionHelpText(options::OPT_ast_print)));
static cl::opt<std::string> ASTDumpFilter("ast-dump-filter",
    cl::desc(Options->getOptionHelpText(options::OPT_ast_dump_filter)));
static cl::opt<bool> ASTInline("inline",
    cl::desc("Inline all non entry point device driver functions"));

static cl::OptionCategory ToolCategory("Chauffeur options");

// These three are to pass on info about which entry points to make calls
// to, and whether to mark the file as true (no bug) or false (buggy)
static cl::opt<std::string> Ep1("ep1",
                                cl::desc("the first entry point to call"),
                                cl::cat(ToolCategory));
static cl::opt<std::string> Ep2("ep2",
                                cl::desc("the second entry point to call"),
                                cl::cat(ToolCategory));
static cl::opt<std::string> Bug("hasBug",
                                cl::desc("does the benchmark have a bug"),
                                cl::cat(ToolCategory));

namespace chauffeur
{

  std::vector<string> split(const std::string &s, char delim) {
    std::vector<string> elems;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
  }

  class ParseDriverASTAction : public ASTFrontendAction
  {
  public:
      virtual ASTConsumer *CreateASTConsumer(CompilerInstance &CI, StringRef file)
      {
        if (ASTList)
          return clang::CreateASTDeclNodeLister();
        if (ASTDump)
          return clang::CreateASTDumper(ASTDumpFilter);
        if (ASTPrint)
          return clang::CreateASTPrinter(&llvm::outs(), ASTDumpFilter);
        if (ASTInline)
          llvm::errs() << "test" << "\n";
        return new ParseDriverConsumer(&CI, ASTInline);
      }
  };
}

int main(int argc, const char **argv)
{
  CommonOptionsParser op(argc, argv, ToolCategory);
  std::vector<std::string> path = chauffeur::split(op.getSourcePathList()[0], '/');
        
  ClangTool Tool(op.getCompilations(), op.getSourcePathList());
  chauffeur::FileName = op.getSourcePathList()[0].substr(0, op.getSourcePathList()[0].find_last_of("."));
  chauffeur::Folder = op.getSourcePathList()[0].substr(0, op.getSourcePathList()[0].find_last_of("/"));
  chauffeur::Ep1 = Ep1;
  chauffeur::Ep2 = Ep2;
  chauffeur::Bug = Bug;
  chauffeur::Driver = path[path.size()-2];
  chauffeur::Group = path[path.size()-3];
  return Tool.run(newFrontendActionFactory<chauffeur::ParseDriverASTAction>().get());
}
