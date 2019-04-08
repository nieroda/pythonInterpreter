#include <iostream>

#include "SymTab.hpp"

// https://stackoverflow.com/questions/41871115/why-would-i-stdmove-an-stdshared-ptr
void SymTab::createEntryFor(std::string vName, int value) {

    if (debug)
        std::cout << "SymTab::createEntryFor(INT) ->" << value << "<-" << std::endl;

    auto descriptor = std::make_shared<NumberDescriptor>(TypeDescriptor::INTEGER);
    descriptor->_value.intValue = value;
    symTab[vName] = std::move(descriptor);
}

void SymTab::createEntryFor(std::string vName, double value) {

    if (debug)
        std::cout << "SymTab::createEntryFor(DOUBLE) ->" << value << "<-" << std::endl;

    auto descriptor = std::make_shared<NumberDescriptor>(TypeDescriptor::DOUBLE);
    descriptor->_value.doubleValue = value;
    symTab[vName] = std::move(descriptor);
}

void SymTab::createEntryFor(std::string vName, bool value) {

    if (debug)
        std::cout << "SymTab::createEntryFor(BOOL) ->" << value << "<-" << std::endl;

    auto descriptor = std::make_shared<NumberDescriptor>(TypeDescriptor::BOOL);
    descriptor->_value.boolValue = (int) value;
    symTab[vName] = std::move(descriptor);
}

void SymTab::createEntryFor(std::string vName, std::string value) {

    if (debug)
        std::cout << "SymTab::createEntryFor(STRING) ->" << value << "<-" << std::endl;

    auto descriptor = std::make_shared<StringDescriptor>(TypeDescriptor::STRING);
    descriptor->_stringValue = value;
    symTab[vName] = std::move(descriptor);
}

void SymTab::setValueFor(std::string vName, std::shared_ptr<TypeDescriptor> sp) {
    symTab[vName] = std::move(sp);
}

bool SymTab::isDefined(std::string vName) {
    return symTab.find(vName) != symTab.end();
}

bool SymTab::erase(std::string vName) {
    if (isDefined(vName)) {
        auto iterator = symTab.find(vName);
        symTab.erase(iterator);
        return true;
    }
    return false;
}

// std::weak_ptr<TypeDescriptor> SymTab::getValueFor(std::string vName) {

//     if ( !isDefined(vName) ) {
//         std::cout << "SymTab::getValueFor: " << vName << " has not been defined.\n";
//         exit(1);
//     }

//     if (debug)
//         std::cout << "SymTab::getValueFor: " << vName << "\n";

//     return symTab.find(vName)->second;

// }

TypeDescriptor *SymTab::getValueFor(std::string vName) {

    if ( !isDefined(vName) ) {
        std::cout << "SymTab::getValueFor: " << vName << " has not been defined.\n";
        exit(1);
    }

    if (debug)
        std::cout << "SymTab::getValueFor: " << vName << "\n";

    return symTab.find(vName)->second.get();

}