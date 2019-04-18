#ifndef EXPRINTER_SYMTAB_HPP
#define EXPRINTER_SYMTAB_HPP

#include <string>
#include <stack>
#include <map>

#include "Descriptor.hpp"

// This is a flat and integer-based symbol table. It allows for variables to be
// initialized, determines if a give variable has been defined or not, and if
// a variable has been defined, it returns its value.

class SymTab {
public:
    bool isDefined(std::string vName);
    bool erase(std::string vName);
    void createEntryFor(std::string, int);
    void createEntryFor(std::string, double);
    void createEntryFor(std::string, bool);
    void createEntryFor(std::string, std::string);
    void addDescriptor(std::string,  std::shared_ptr<NumberDescriptor>);

    void setValueFor(std::string, std::shared_ptr<TypeDescriptor>);
    // int getValueFor(std::string vName);

    TypeDescriptor *getValueFor(std::string);

private:

    std::map<
        std::string, 
        std::shared_ptr<TypeDescriptor>
    > globalSymTab;

    std::stack<
        std::map< std::string, std::shared_ptr<TypeDescriptor>
	>> symTab;
};

#endif //EXPRINTER_SYMTAB_HPP

// STD MOVE
// template<class _Ty> inline
//   constexpr typename remove_reference<_Ty>::type&&
//     move(_Ty&& _Arg) _NOEXCEPT
//   {
//     return (static_cast<typename remove_reference<_Ty>::type&&>(_Arg));
//   }
 
