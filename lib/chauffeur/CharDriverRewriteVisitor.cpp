//
// Copyright (c) 2014-2015 Pantazis Deligiannis (p.deligiannis@imperial.ac.uk)
// This file is distributed under the MIT License. See LICENSE for details.
//

#include "chauffeur/Chauffeur.h"
#include "chauffeur/CharDriverRewriteVisitor.h"

namespace chauffeur
{
  using namespace std;

  void CharDriverRewriteVisitor::InstrumentEntryPoints(FunctionDecl* funcDecl, string fdFile)
  {
    // Remove static keyword from all entry points
    if (funcDecl->getStorageClass() == SC_Static)
      RW.RemoveText(funcDecl->getInnerLocStart(), 7);
    return;
  }

  // In this case, funcDecl WILL be the init function (selected by if statement in AbstractDriverRewriteVisitor)
  void CharDriverRewriteVisitor::CreateCheckerFunction(FunctionDecl* funcDecl, string fdFile)
  {
    string device_str;
    string shared_struct_str;

    // Find the beginning and end of the file
    FileID fileId = Context->getSourceManager().getFileID(funcDecl->getLocation());
    SourceLocation top = Context->getSourceManager().getLocForStartOfFile(fileId);
    SourceLocation loc = Context->getSourceManager().getLocForEndOfFile(fileId);

    // Add smack and pthread include
    RW.InsertText(top, "#include <pthread.h>\n", true, true);

    // The rest of the rewrite goes at the bottom
    RW.InsertText(loc, "\n", true, true);

    // Get a list of the entry points for the driver
    auto entry_points = DI->getInstance().GetEntryPoints();

    // Write a numbered declaration inode and file for each entry point
    RW.InsertText(loc, "// Declare values needed by entry point wrappers\n", true, true);
    for (unsigned long i = 0; i < entry_points.size(); i++)
      {
        RW.InsertText(loc, "struct inode *whoop_inode_" + std::to_string(i) + ";\n", true, true);
        RW.InsertText(loc, "struct file *whoop_file_" + std::to_string(i) + ";\n", true, true);
      }

    // Write declarations for other types of required entry point params
    RW.InsertText(loc, "struct pci_dev *whoop_pci_dev;\n", true, true);
    RW.InsertText(loc, "const char *whoop_buf;\n", true, true);
    RW.InsertText(loc, "struct platform_device *whoop_platform_device;\n", true, true);
    RW.InsertText(loc, "struct vm_area_struct *whoop_vm_area_struct;\n", true, true);
    RW.InsertText(loc, "struct cx_dev *whoop_cx_dev;\n\n", true, true);

    RW.InsertText(loc, "poll_table *whoop_poll_table;\n\n", true, true);
    
    RW.InsertText(loc, "loff_t *whoop_loff_t;\n", true, true);
    RW.InsertText(loc, "int whoop_int;\n\n", true, true);

    int counter = 0;
    string pthread_ts = "";
    string creates = "";
    string joins = "";
    // Write one wrapper for a call to each entry point
    RW.InsertText(loc, "// Pthread wrappers for entry points\n", true, true);
    for(auto i = entry_points.rbegin(); i != entry_points.rend(); i++)
      { 
        if (i->first == Ep1 || i->first == Ep2) {

          if(Ep1 == Ep2) {
            // Add a pthread_t struct to the list of pthread_t's
            pthread_ts += "\tpthread_t pthread_t_" + i->first + "_1;\n";
            pthread_ts += "\tpthread_t pthread_t_" + i->first + "_2;\n";
            // Add a pthread_create call to the list of pthread_create's
            creates += "\tpthread_create(&pthread_t_" + i->first + "_1";
            creates += ", NULL, whoop_wrapper_" + i->first + "_1, NULL);\n";
            creates += "\tpthread_create(&pthread_t_" + i->first + "_2";
            creates += ", NULL, whoop_wrapper_" + i->first + "_2, NULL);\n";
            // Add a pthread_join call to the list of pthread_join's
            joins += "\tpthread_join(pthread_t_" + i->first + "_1, NULL);\n";
            joins += "\tpthread_join(pthread_t_" + i->first + "_2, NULL);\n";
          } else {
            // Add a pthread_t struct to the list of pthread_t's
            pthread_ts += "\tpthread_t pthread_t_" + i->first + ";\n";
            // Add a pthread_create call to the list of pthread_create's
            creates += "\tpthread_create(&pthread_t_" + i->first;
            creates += ", NULL, whoop_wrapper_" + i->first + ", NULL);\n";
            // Add a pthread_join call to the list of pthread_join's
            joins += "\tpthread_join(pthread_t_" + i->first + ", NULL);\n";
          }
        }

        // Build wrapper text for current entry point
        // If entry point pair are both the same, create separate wrapper,
        //   in case we need to have different input args
        string entry_point_call, entry_point_call_2;
        if (Ep1 == Ep2 && i->first == Ep1) {
          entry_point_call   = "void *whoop_wrapper_" + i->first + "_1(void* args)\n{\n";
          entry_point_call_2 = "void *whoop_wrapper_" + i->first + "_2(void* args)\n{\n";
        } else {
          entry_point_call   = "void *whoop_wrapper_" + i->first + "(void* args)\n{\n";
        }
        entry_point_call   += "\t" + i->first + "(";
        entry_point_call_2 += "\t" + i->first + "(";

        // Write the parameters, based on entry point signature.
        for(auto j = i->second.begin(); j != i->second.end(); j++)
          {
            if (*j == "void") {
              entry_point_call   += ", ";
              entry_point_call_2 += ", ";
            } else if (*j == "void *") {
              entry_point_call   += "NULL, ";
              entry_point_call_2 += "NULL, ";
            } else if (*j == "u64 *") {
              entry_point_call   += "NULL, ";
              entry_point_call_2 += "NULL, ";
            } else if (*j == "u8 *") {
              entry_point_call   += "NULL, ";
              entry_point_call_2 += "NULL, ";
            } else if (*j == "struct pci_dev *") {
              entry_point_call   += "whoop_pci_dev, ";
              entry_point_call_2 += "whoop_pci_dev, ";
            //TODO: if calling same entry point twice, should inode/fail be different?
            } else if (*j == "struct inode *") {
              entry_point_call   += "whoop_inode_" + std::to_string(counter) + ", ";
              entry_point_call_2 += "whoop_inode_" + std::to_string(counter) + ", ";
            } else if (*j == "struct file *") {
              entry_point_call   += "whoop_file_" + std::to_string(counter) + ", ";
              entry_point_call_2 += "whoop_file_" + std::to_string(counter) + ", ";
            } else if (*j == "char *") {
              entry_point_call   += "whoop_buf, ";
              entry_point_call_2 += "whoop_buf, ";
            } else if (*j == "const char *") {
              entry_point_call   += "whoop_buf, ";
              entry_point_call_2 += "whoop_buf, ";
            } else if (*j == "loff_t *") {
              entry_point_call   += "whoop_loff_t, ";
              entry_point_call_2 += "whoop_loff_t, ";
            } else if (*j == "loff_t") {
              entry_point_call   += "&whoop_loff_t, ";
              entry_point_call_2 += "&whoop_loff_t, ";
            } else if (*j == "poll_table *") {
              entry_point_call   += "whoop_poll_table, ";
              entry_point_call_2 += "whoop_poll_table, ";
            } else if (*j == "struct platform_device *") {
              entry_point_call   += "whoop_platform_device, ";
              entry_point_call_2 += "whoop_platform_device, ";
            } else if (*j == "struct vm_area_struct *") {
              entry_point_call   += "whoop_vm_area_struct, ";
              entry_point_call_2 += "whoop_vm_area_struct, ";
            } else if (*j == "struct cx_dev *") {
              entry_point_call   += "whoop_cx_dev, ";
              entry_point_call_2 += "whoop_cx_dev, ";
            } else if (*j == "size_t") {
              entry_point_call   += "whoop_int, ";
              entry_point_call_2 += "whoop_int, ";
            } else if (*j == "int") {
              entry_point_call   += "whoop_int, ";
              entry_point_call_2 += "whoop_int, ";
            } else if (*j == "unsigned int") {
              entry_point_call   += "whoop_int, ";
              entry_point_call_2 += "whoop_int, ";
            } else if (*j == "long") {
              entry_point_call   += "whoop_int, ";
              entry_point_call_2 += "whoop_int, ";
            } else if (*j == "unsigned long") {
              entry_point_call   += "whoop_int, ";
              entry_point_call_2 += "whoop_int, ";
            } else if (*j == "u32") {
              entry_point_call += "whoop_int, ";
              entry_point_call_2 += "whoop_int, ";
            } else {
              entry_point_call   += *j + ", ";
              entry_point_call_2 += *j + ", ";
            }
          }

        // If there was at least one param, take off last comma and space
        if ( i->second.size() > 0)
          {
            entry_point_call.resize(entry_point_call.size() - 2);
          }
        if ( i->second.size() > 0)
          {
            entry_point_call_2.resize(entry_point_call_2.size() - 2);
          }

        entry_point_call   += ");\n}\n\n";
        entry_point_call_2 += ");\n}\n\n";

        RW.InsertText(loc, entry_point_call, true, true);
        if(Ep1 == Ep2 && Ep1 == i->first) {
          RW.InsertText(loc, entry_point_call_2, true, true);
        }
        counter++;
      }

    // Write main signature
    RW.InsertText(loc, "void main(", true, true);
    // Loop through params required by the init function
    // Add params of init function as params for checker function
    map<string, string> func_params;
    for (auto i = funcDecl->param_begin(), e = funcDecl->param_end(); i != e; ++i)
      {
        ValueDecl *paramVal = cast<ValueDecl>(*i);
        NamedDecl *paramNam = cast<NamedDecl>(*i);

        string paramType = paramVal->getType().getAsString(Context->getPrintingPolicy());
        string paramName = paramNam->getNameAsString();

        func_params[paramType] = paramName;

        // Will these commas really work?  
        if (i == funcDecl->param_begin())
          RW.InsertText(loc, paramType + " " + paramName + ", ", true, true);
        else
          RW.InsertText(loc, paramType + " " + paramName, true, true);
      }

    // Write checker signature close paren, and body open brace
    RW.InsertText(loc, ")\n", true, true);
    RW.InsertText(loc, "{\n", true, true);


    RW.InsertText(loc, "\t// Instantiate values required by entry poitns\n", true, true);

    // Allocate an inode and file for each entry point
    for (unsigned long i = 0; i < entry_points.size(); i++)
      {
        RW.InsertText(loc, "\twhoop_inode_" + std::to_string(i) + " = (struct inode *) malloc(sizeof(struct inode));\n", true, true);
        RW.InsertText(loc, "\twhoop_file_" + std::to_string(i) + " = (struct file *) malloc(sizeof(struct file));\n", true, true);
      }

    // Write allocations for other types of required entry point params
    RW.InsertText(loc, "\twhoop_pci_dev = (struct pci_dev *) malloc(sizeof(struct pci_dev));\n", true, true);
    RW.InsertText(loc, "\twhoop_buf = (char *) malloc(sizeof(char));\n", true, true);
    RW.InsertText(loc, "\twhoop_platform_device = (struct platform_device *) malloc(sizeof(struct platform_device));\n", true, true);
    RW.InsertText(loc, "\twhoop_vm_area_struct = (struct vm_area_struct *) malloc(sizeof(struct vm_area_struct));\n", true, true);
    RW.InsertText(loc, "\twhoop_cx_dev = (struct cx_dev *) malloc(sizeof(struct cx_dev));\n\n", true, true);

    RW.InsertText(loc, "\twhoop_poll_table = (poll_table *) malloc(sizeof(poll_table));\n\n", true, true);
    
    RW.InsertText(loc, "\twhoop_loff_t = (loff_t *) malloc(sizeof(loff_t));\n", true, true);
    RW.InsertText(loc, "\twhoop_int = __VERIFIER_nondet_int();\n", true, true);
    RW.InsertText(loc, "\t__VERIFIER_assume(whoop_int >= 0);\n\n", true, true);

    // Call init function
    //RW.InsertText(loc, "\t// Call module_init function\n", true, true);
    //RW.InsertText(loc, "\t_whoop_init();\n\n", true, true);

    // Insert pthread_t decls, then pthread_create & pthread_join calls
    RW.InsertText(loc, "\t// Declare pthread_t's\n", true, true);
    RW.InsertText(loc, pthread_ts + "\n", true, true);
    RW.InsertText(loc, "\t// Create pthread threads\n", true, true);
    RW.InsertText(loc, creates + "\n", true, true);
    RW.InsertText(loc, "\t// Wait for threads to finish\n", true, true);
    RW.InsertText(loc, joins + "\n", true, true);

    // Call cleanup function
    //RW.InsertText(loc, "\t// Call module_cleanup function\n", true, true);
    //RW.InsertText(loc, "\t_whoop_exit();\n\n", true, true);

    RW.InsertText(loc, "}\n", true, true);
  }

  string CharDriverRewriteVisitor::GetSharedStructStr(CallExpr *callExpr)
  {
    return "";
  }
}
