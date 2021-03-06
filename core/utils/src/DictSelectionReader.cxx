#include "DictSelectionReader.h"

#include "clang/AST/AST.h"

#include "ClassSelectionRule.h"
#include "SelectionRules.h"
#include "TMetaUtils.h"

#include "RootMetaSelection.h"

#include <iostream>
#include <sstream>

//______________________________________________________________________________
DictSelectionReader::DictSelectionReader(SelectionRules& selectionRules,
                                         const clang::ASTContext &C):
   fSelectionRules(selectionRules),fIsFirstPass(true)
{
   clang::TranslationUnitDecl* translUnitDecl = C.getTranslationUnitDecl();
   // Inspect the AST
   TraverseDecl(translUnitDecl);

   // Now re-inspect the AST to find autoselected classes (double-tap)
   fIsFirstPass=false;
   if (!fTemplateInstanceNamePatternsArgsToKeep.empty() ||
       !fAutoSelectedClassFieldNames.empty())
      TraverseDecl(translUnitDecl);
   
   // Now push all the selection rules
   for (llvm::StringMap<ClassSelectionRule>::iterator it=fClassNameSelectionRuleMap.begin();
        it != fClassNameSelectionRuleMap.end(); ++it){
      fSelectionRules.AddClassSelectionRule(it->second);
      
      }
        
}

//______________________________________________________________________________
/**
 * Check that the recordDecl is enclosed in the ROOT::Meta::Selection namespace,
 * excluding the portion dedicated the definition of the syntax, which is part
 * of ROOT, not of the user code.
 * If performance is needed, an alternative approach to string comparisons
 * could be adopted. One could use for example hashes of strings in first
 * approximation.
 **/
bool DictSelectionReader::InSelectionNamespace(const clang::RecordDecl& recordDecl,
                                                const std::string& className)
{   
   // If it's not contained by 2 namespaces, drop it.
   std::list<std::pair<std::string,bool> > enclosingNamespaces;
   ROOT::TMetaUtils::ExtractEnclosingNameSpaces(recordDecl,enclosingNamespaces);
   
   const unsigned int nNs = enclosingNamespaces.size();
   if (nNs < 3)
      return false;

   if (enclosingNamespaces.back().second || // is inline namespace
       enclosingNamespaces.back().first != "ROOT")
      return false;

   enclosingNamespaces.pop_back();
   if (enclosingNamespaces.back().second || // is inline namespace
       enclosingNamespaces.back().first != "Meta")
      return false;

   enclosingNamespaces.pop_back();   
   if (enclosingNamespaces.back().second || // is inline namespace
       enclosingNamespaces.back().first != "Selection")
      return false;     
  
   // Exclude the special names identifying the entities of the selection syntax   
   if (className!= "" && ( className.find("MemberAttributes") == 0 ||
                           className.find("ClassAttributes") == 0 ||
                           className.find("Keep") == 0))
       return false;
   
   return true;
  
}

//______________________________________________________________________________
/**
 * Get the pointer to the template arguments list. Return zero if not available.
 **/
const clang::TemplateArgumentList* DictSelectionReader::GetTmplArgList(const clang::CXXRecordDecl& cxxRcrdDecl)
{
   const clang::ClassTemplateSpecializationDecl* tmplSpecDecl =
   llvm::dyn_cast<clang::ClassTemplateSpecializationDecl> (&cxxRcrdDecl);

   if (!tmplSpecDecl) return 0;

   return  &tmplSpecDecl->getTemplateArgs();
}  

//______________________________________________________________________________
/**
 * Extract the value of the integral template parameter of a CXXRecordDecl when
 * it has a certain name. If nothing can be extracted, the value of @c zero
 * is returned.
 **/
template<class T>
unsigned int DictSelectionReader::ExtractTemplateArgValue(const T& myClass,
                                                          const std::string& pattern)
{
   const clang::RecordDecl* rcrdDecl =
                   ROOT::TMetaUtils::GetUnderlyingRecordDecl(myClass.getType());
   const clang::CXXRecordDecl* cxxRcrdDecl =
                                 llvm::dyn_cast<clang::CXXRecordDecl>(rcrdDecl);

   if (!cxxRcrdDecl) return 0;
   
   const clang::TemplateArgumentList* tmplArgs = GetTmplArgList(*cxxRcrdDecl);
   if (!tmplArgs) return 0;
   
   if ( std::string::npos == cxxRcrdDecl->getNameAsString().find(pattern))
      return 0;
   
   return tmplArgs->get(0).getAsIntegral().getLimitedValue();
}

//______________________________________________________________________________
/**
 * Loop over the class filelds and take actions according to their properties
 *    1. Insert a field selection rule marking a member transient
 *    2. Store in a map the name of the field the type of which should be
 * autoselected. The key is the name of the class and the value the name of the
 * field. This information is used in the second pass.
 **/
void DictSelectionReader::ManageFields(const clang::RecordDecl& recordDecl,
                   const std::string& className,
                   ClassSelectionRule& csr)
{
   // Iterate on the members to see if
   // 1) They are transient
   // 2) They imply further selection

   std::string fieldClassName;
   
   for (clang::RecordDecl::field_iterator fieldsIt = recordDecl.field_begin();
        fieldsIt!=recordDecl.field_end();++fieldsIt){

      unsigned int attrCode = ExtractTemplateArgValue(**fieldsIt,"MemberAttributes");

      if (attrCode == ROOT::Meta::Selection::kMemberNullProperty)
         continue;
      
      if (attrCode & ROOT::Meta::Selection::kTransient){
         VariableSelectionRule vsr(BaseSelectionRule::kYes);
         vsr.SetAttributeValue("name", fieldsIt->getNameAsString());
         vsr.SetAttributeValue("transient", "true");
         csr.AddFieldSelectionRule(vsr);
      }

      if (attrCode & ROOT::Meta::Selection::kAutoSelected)
         fAutoSelectedClassFieldNames[className].insert(fieldsIt->getNameAsString());

   } // end loop on fields
   
}

//______________________________________________________________________________
/**
 * Manage the loop over the base classes.
 * Initially, the class attributes are identified and selection rules filled
 * if:
 *    1. The class is not splittable
 * Then we look for the traits pointing to the need of hiding template
 * arguments. This information is stored in the form of a list of pairs, where
 * the first argument is the pattern of the template instance to match and
 * the second one the number of arguments to be skipped. This information is
 * used during the second pass.
 **/
void DictSelectionReader::ManageBaseClasses(const clang::CXXRecordDecl& cxxRcrdDecl,
                                             const std::string& className,
                                             ClassSelectionRule& csr)
{
   // Check the traits of the class. Useful information may be there
   //extract mothers, make a switchcase:
   //1) templates args are to be skipped
   //2) There are properties. Make a loop. make a switch:
   //  2a) Is splittable

   for( clang::CXXRecordDecl::base_class_const_iterator baseIt = cxxRcrdDecl.bases_begin();
       baseIt !=  cxxRcrdDecl.bases_end(); baseIt++){
            
      if (unsigned int attrCode = ExtractTemplateArgValue(*baseIt,"ClassAttributes") ){
         if (attrCode & ROOT::Meta::Selection::kNonSplittable)
            csr.SetAttributeValue("nonSplittable", "true");
      }
      if ( unsigned int nArgsToKeep = ExtractTemplateArgValue(*baseIt,"Keep") ){      
         std::string pattern = className.substr(0,className.find_first_of("<"));
         // Fill the structure holding the template and the number of args to
         // skip
         fTemplateInstanceNamePatternsArgsToKeep.push_back(std::make_pair(pattern,nArgsToKeep));
      }

   } // end loop on base classes
}

//______________________________________________________________________________
/**
 * Manage the first pass over the AST, inspecting only nodes which are within
 * the selection namespace. Selection rules are directly filled as well as
 * data sructures re-used during the second pass.
 **/
bool DictSelectionReader::FirstPass(const clang::RecordDecl& recordDecl)
{

   std::string className;
   ROOT::TMetaUtils::GetQualifiedName(className,
                                      *recordDecl.getTypeForDecl(),
                                      recordDecl);
      
   // Strip ROOT::Meta::Selection
   className.replace(0,23,"");
   
   if ( ! InSelectionNamespace(recordDecl,className))
         return true;
   
   if ( !fSelectedRecordDecls.insert(&recordDecl).second )
      return true;
  
   ClassSelectionRule csr(BaseSelectionRule::kYes);
   csr.SetAttributeValue("name",className);

   ManageFields(recordDecl, className, csr);

   if (const clang::CXXRecordDecl* cxxRcrdDecl =
                          llvm::dyn_cast<clang::CXXRecordDecl>(&recordDecl)){
      ManageBaseClasses(*cxxRcrdDecl,className,csr);
   }

   // Finally add the selection rule
   fClassNameSelectionRuleMap[className]=csr;
   //fSelectionRules.AddClassSelectionRule(csr);


   return true;

}

//______________________________________________________________________________
/**
 * Second pass through the AST. Two operations are performed:
 *    1. Selection rules for classes to be autoselected are created. The
 * algorithm works as follows: the members of the classes matching the name of
 * the classes which contained autoselected members in the selection namespace
 * are inspected. If a field with the same name of the one which was
 * autoselected a selection rule based on its typename is built.
 *    2. If a class is found which is a @c TemplateSpecialisationDecl its
 * name is checked to match one of the patterns identified during the first
 * pass. If a match is found, a property is added to the selection rule with
 * the number of template arguments to keep in order to percolate this
 * information down to the @c AnnotatedRecordDecl creation which happens in the
 * @c RScanner .
 **/
bool DictSelectionReader::SecondPass(const clang::RecordDecl& recordDecl)
{
   // No interest if we are in the selction namespace
   if ( InSelectionNamespace(recordDecl))
      return true;

   std::string className;
   ROOT::TMetaUtils::GetQualifiedName(className,
                                      *recordDecl.getTypeForDecl(),
                                      recordDecl);

   // If the class is not among those which have fields the type of which are to
   // be autoselected
   if (0 != fAutoSelectedClassFieldNames.count(className)){
      // Iterate on fields. If the name of the field is among the ones the types
      // of which should be autoselected, add a class selection rule
      std::string typeName;
      clang::ASTContext& C = recordDecl.getASTContext();
      for (clang::RecordDecl::field_iterator filedsIt = recordDecl.field_begin();
         filedsIt!=recordDecl.field_end();++filedsIt){
         const std::string fieldName (filedsIt->getNameAsString());
         if (0 == fAutoSelectedClassFieldNames[className].count(fieldName))
            continue;
         ClassSelectionRule aSelCsr(BaseSelectionRule::kYes);
         ROOT::TMetaUtils::GetFullyQualifiedTypeName(typeName,
                                                    filedsIt->getType(),
                                                    C);
         aSelCsr.SetAttributeValue("name",typeName);
         fSelectionRules.AddClassSelectionRule(aSelCsr);
      }
   }
   
   

   // If the class is a template instantiation and its name matches one of the
   // patterns

   // We don't want anything different from templ specialisations
   if (!llvm::isa<clang::ClassTemplateSpecializationDecl>(recordDecl)) return true;

   unsigned int nArgsToKeep;
   std::list<std::pair<std::string,unsigned int> >::iterator it = fTemplateInstanceNamePatternsArgsToKeep.begin();
   for (;it !=fTemplateInstanceNamePatternsArgsToKeep.end(); ++it){
      std::string& pattern = it->first;
      nArgsToKeep = it->second;
      // Check if we have to add a selection rule for this class
      if (className.find(pattern) == 0){    
         std::ostringstream oss;
         oss << nArgsToKeep;
         fClassNameSelectionRuleMap[className].SetAttributeValue("KeepFirstTemplateArguments",oss.str());
      }
   }
   return true;

}

//______________________________________________________________________________
bool DictSelectionReader::VisitRecordDecl(clang::RecordDecl* recordDecl)
{

   if (fIsFirstPass)
      return FirstPass(*recordDecl);
   else 
      return SecondPass(*recordDecl);
}

   
