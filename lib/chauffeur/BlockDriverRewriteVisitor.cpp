//
// Copyright (c) 2014-2015 Pantazis Deligiannis (p.deligiannis@imperial.ac.uk)
// This file is distributed under the MIT License. See LICENSE for details.
//

#include "chauffeur/Chauffeur.h"
#include "chauffeur/BlockDriverRewriteVisitor.h"

namespace chauffeur
{
  using namespace std;

  void BlockDriverRewriteVisitor::InstrumentEntryPoints(FunctionDecl* funcDecl, string fdFile)
  {
    if (funcDecl->getStorageClass() == SC_Static)
      RW.RemoveText(funcDecl->getInnerLocStart(), 7);
    return;
  }

  void BlockDriverRewriteVisitor::CreateCheckerFunction(FunctionDecl* funcDecl, string fdFile)
  {
    string device_str;
    string shared_struct_str;

    FileID fileId = Context->getSourceManager().getFileID(funcDecl->getLocation());
    SourceLocation loc = Context->getSourceManager().getLocForEndOfFile(fileId);

    RW.InsertText(loc, "\n", true, true);
    RW.InsertText(loc, "void whoop$checker(", true, true);

    map<string, string> func_params;
    for (auto i = funcDecl->param_begin(), e = funcDecl->param_end(); i != e; ++i)
    {
      ValueDecl *paramVal = cast<ValueDecl>(*i);
      NamedDecl *paramNam = cast<NamedDecl>(*i);

      string paramType = paramVal->getType().getAsString(Context->getPrintingPolicy());
      string paramName = paramNam->getNameAsString();

      func_params[paramType] = paramName;

      if (i == funcDecl->param_begin())
        RW.InsertText(loc, paramType + " " + paramName + ", ", true, true);
      else
        RW.InsertText(loc, paramType + " " + paramName, true, true);
    }

    RW.InsertText(loc, ")\n", true, true);
    RW.InsertText(loc, "{\n", true, true);

    auto entry_points = DI->getInstance().GetEntryPoints();

    for (unsigned long i = 0; i < entry_points.size(); i++)
    {
      RW.InsertText(loc, "\tstruct inode *whoop_inode$" + std::to_string(i) + " = (struct inode *) malloc(sizeof(struct inode));\n", true, true);
      RW.InsertText(loc, "\tstruct file *whoop_file$" + std::to_string(i) + " = (struct file *) malloc(sizeof(struct file));\n", true, true);
      RW.InsertText(loc, "\tstruct block_device *whoop_bdev$" + std::to_string(i) + " = (struct block_device *) malloc(sizeof(struct block_device));\n", true, true);
    }

    RW.InsertText(loc, "\tstruct pci_dev *whoop_pci_dev = (struct pci_dev *) malloc(sizeof(struct pci_dev));\n", true, true);
    RW.InsertText(loc, "\tstruct platform_device *whoop_platform_device = (struct platform_device *) malloc(sizeof(struct platform_device));\n", true, true);
    RW.InsertText(loc, "\tstruct cdrom_device_info *whoop_cdrom_device_info = (struct cdrom_device_info *) malloc(sizeof(struct cdrom_device_info));\n", true, true);
    RW.InsertText(loc, "\tstruct cdrom_multisession *whoop_cdrom_multisession = (struct cdrom_multisession *) malloc(sizeof(struct cdrom_multisession));\n", true, true);
    RW.InsertText(loc, "\tstruct ps3_system_bus_device *whoop_ps3_system_bus_device = (struct ps3_system_bus_device *) malloc(sizeof(struct ps3_system_bus_device));\n", true, true);
    RW.InsertText(loc, "\tstruct hd_geometry *whoop_geo = (struct hd_geometry *) malloc(sizeof(struct hd_geometry));\n", true, true);
    RW.InsertText(loc, "\tconst char *whoop_buf = (char *) malloc(sizeof(char));\n", true, true);
    RW.InsertText(loc, "\tstruct gendisk *whoop_disk = (struct gendisk *) malloc(sizeof(struct gendisk));\n\n", true, true);

    RW.InsertText(loc, "\tint whoop_int = __SMACK_nondet();\n", true, true);
    RW.InsertText(loc, "\t__SMACK_code(\"assume @ >= @;\", whoop_int, 0);\n\n", true, true);

    int counter = 0;
    for(auto i = entry_points.rbegin(); i != entry_points.rend(); i++)
    {
      string entry_point_call;
      entry_point_call = "" + i->first + "(";

      for(auto j = i->second.begin(); j != i->second.end(); j++)
      {
        if (*j == "void")
          entry_point_call += "";
        else if (*j == "void *")
          entry_point_call += "NULL, ";
        else if (*j == "u64 *")
          entry_point_call += "NULL, ";
        else if (*j == "u8 *")
          entry_point_call += "NULL, ";
        else if (*j == "struct pci_dev *")
          entry_point_call += "whoop_pci_dev, ";
        else if (*j == "struct block_device *")
          entry_point_call += "whoop_bdev$" + std::to_string(counter) + ", ";
        else if (*j == "struct platform_device *")
          entry_point_call += "whoop_platform_device, ";
        else if (*j == "struct cdrom_device_info *")
          entry_point_call += "whoop_cdrom_device_info, ";
        else if (*j == "struct cdrom_multisession *")
          entry_point_call += "whoop_cdrom_multisession, ";
        else if (*j == "struct ps3_system_bus_device *")
          entry_point_call += "whoop_ps3_system_bus_device, ";
        else if (*j == "struct hd_geometry *")
          entry_point_call += "whoop_geo, ";
        else if (*j == "struct inode *")
          entry_point_call += "whoop_inode$" + std::to_string(counter) + ", ";
        else if (*j == "struct file *")
          entry_point_call += "whoop_file$" + std::to_string(counter) + ", ";
        else if (*j == "struct gendisk *")
          entry_point_call += "whoop_disk, ";
        else if (*j == "char *")
          entry_point_call += "whoop_buf, ";
        else if (*j == "const char *")
          entry_point_call += "whoop_buf, ";
        else if (*j == "size_t")
          entry_point_call += "whoop_int, ";
        else if (*j == "int")
          entry_point_call += "whoop_int, ";
        else if (*j == "unsigned int")
          entry_point_call += "whoop_int, ";
        else if (*j == "long")
          entry_point_call += "whoop_int, ";
        else if (*j == "unsigned long")
          entry_point_call += "whoop_int, ";
        else if (*j == "u32")
          entry_point_call += "whoop_int, ";
        else if (*j == "fmode_t")
          entry_point_call += "whoop_int, ";
        else
          entry_point_call += *j + ", ";
      }

      if (entry_point_call != i->first + "(")
      {
        entry_point_call.resize(entry_point_call.size() - 2);
      }

      RW.InsertText(loc, "\t" + entry_point_call + ");\n", true, true);
      counter++;
    }

    RW.InsertText(loc, "}", true, true);
  }

  string BlockDriverRewriteVisitor::GetSharedStructStr(CallExpr *callExpr)
  {
    return "";
  }
}
