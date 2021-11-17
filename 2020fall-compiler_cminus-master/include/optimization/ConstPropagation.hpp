#ifndef CONSTPROPAGATION_HPP
#define CONSTPROPAGATION_HPP
#include "PassManager.hpp"
#include "Constant.h"
#include "Instruction.h"
#include "Module.h"

#include "Value.h"
#include "IRBuilder.h"
#include <vector>
#include <stack>
#include <unordered_map>
// tips: 用来判断value是否为ConstantFP/ConstantInt
ConstantFP* cast_constantfp(Value *value);
ConstantInt* cast_constantint(Value *value);


// tips: ConstFloder类

class ConstIntFolder
{
public:
    ConstIntFolder(Module *m) : module_(m) {}
    ConstantInt *computeint(
        Instruction::OpID op,
        ConstantInt *value1,
        ConstantInt *value2);
    // ...
private:
    Module *module_;
};

class ConstFPFolder
{
public:
    ConstFPFolder(Module *m) : module_(m) {}
    ConstantFP *computefp(
        Instruction::OpID op,
        ConstantFP *value1,
        ConstantFP *value2);
    // ...
private:
    Module *module_;
};

class ConstPropagation : public Pass
{
public:
    ConstPropagation(Module *m) : Pass(m) {}
    void run();
};
#endif