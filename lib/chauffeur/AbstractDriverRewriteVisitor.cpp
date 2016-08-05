//
// Copyright (c) 2014-2015 Pantazis Deligiannis (p.deligiannis@imperial.ac.uk)
// This file is distributed under the MIT License. See LICENSE for details.
//

#include "chauffeur/Chauffeur.h"
#include "chauffeur/AbstractDriverRewriteVisitor.h"

namespace chauffeur
{
  using namespace std;

  bool AbstractDriverRewriteVisitor::VisitFunctionDecl(FunctionDecl* funcDecl)
  {
    string fdFileWithExt = Context->getSourceManager().getFilename(funcDecl->getLocation());
    string fdFile = fdFileWithExt.substr(0, fdFileWithExt.find_last_of("."));

    // If current function is not an entry point & inlining is enabled, inline the function
    if (!(DI->getInstance().ExistsEntryPointWithName(funcDecl->getNameInfo().getName().getAsString())) &&
      DoInline)
    {
      InlineFunctions(funcDecl, fdFile);
    }

    // If current function is an entry point, instrument it
    if (DI->getInstance().ExistsEntryPointWithName(funcDecl->getNameInfo().getName().getAsString()))
    {
      InstrumentEntryPoints(funcDecl, fdFile);
    }

    // If the checker function hasn't been written yet, and current function is init function, create checker
    if (!CheckerExists && funcDecl->getNameInfo().getName().getAsString() == DI->getInstance().GetInitFunction())
    {
      CreateCheckerFunction(funcDecl, fdFile);
      CheckerExists = true;
    }

    return true;
  }

  void AbstractDriverRewriteVisitor::InlineFunctions(FunctionDecl* funcDecl, string fdFile)
  {
    if ((fdFile.size() > 0) && (fdFile.find(FileName) != string::npos))
    {
      if (funcDecl->getStorageClass() == SC_Static)
        RW.ReplaceText(funcDecl->getInnerLocStart(), 6, "static inline");
    }
  }

  void AbstractDriverRewriteVisitor::Finalise()
  {
    string file = Folder;
    file.append("/" + Group + "_" + Driver + "_" + Ep1 + "_" + Ep2 + "_" + NoBug + ".c");

    string error_msg;
    llvm::raw_fd_ostream *fos = new llvm::raw_fd_ostream(file.c_str(), error_msg, llvm::sys::fs::F_None);
    if (!error_msg.empty())
    {
      if (llvm::errs().has_colors()) llvm::errs().changeColor(llvm::raw_ostream::RED);
      llvm::errs() << "error: " << error_msg << "\n";
      if (llvm::errs().has_colors()) llvm::errs().resetColor();
      exit(1);
    }

    fos->SetUnbuffered();
    fos->SetUseAtomicWrites(true);

    raw_ostream *ros = fos;

    RW.getEditBuffer(RW.getSourceMgr().getMainFileID()).write(*ros);
  }

  AbstractDriverRewriteVisitor::~AbstractDriverRewriteVisitor() {}
}
