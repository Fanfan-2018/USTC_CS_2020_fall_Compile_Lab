#include "BasicBlock.h"
#include "Constant.h"
#include "Function.h"
#include "IRBuilder.h"
#include "Module.h"
#include "Type.h"

#include <iostream>
#include <memory>

#ifdef DEBUG  // 用于调试信息,大家可以在编译过程中通过" -DDEBUG"来开启这一选项
#define DEBUG_OUTPUT std::cout << __LINE__ << std::endl;  // 输出行号的简单示例
#else
#define DEBUG_OUTPUT
#endif

#define CONST_INT(num) \
    ConstantInt::get(num, module)

#define CONST_FP(num) \
    ConstantFP::get(num, module) // 得到常数值的表示,方便后面多次用到

int main() {
  auto module = new Module("Cminus code");  // module name是什么无关紧要
  auto builder = new IRBuilder(nullptr, module);
  Type *Int32Type = Type::get_int32_type(module);

  // main函数
  auto mainFun = Function::create(FunctionType::get(Int32Type, {}),
                                  "main", module);
  auto bb = BasicBlock::create(module, "entry", mainFun);
  // BasicBlock的名字在生成中无所谓,但是可以方便阅读
  builder->set_insert_point(bb);

  auto *ArrayType = ArrayType::get(Int32Type, 10);
  auto initializer = ConstantZero::get(Int32Type, module);
  auto a = builder->create_alloca(ArrayType);   // a[10]的存放

  auto a0GEP = builder->create_gep(a, {CONST_INT(0), CONST_INT(0)});  // a[0]
  builder->create_store(CONST_INT(10), a0GEP);// a[0] = 10 store
  auto a1GEP = builder->create_gep(a, {CONST_INT(0), CONST_INT(1)}); // a[1]
  auto load0 = builder->create_load(a0GEP); //load a[0]'s value (10)
  auto mul = builder->create_imul(load0, CONST_INT(2));  // mul = a[0] * 2,the result
  builder->create_store(mul, a1GEP); //store a[1] with the result of mul
  auto load1 = builder->create_load(a1GEP); //load a[1]'s value (a[0] * 2)
  //写ll文件的时候load store没感觉，翻译成cpp文件才发现这么鸡肋。。。    
  builder->create_ret(load1); //ret a[1]*/
  std::cout << module->print();
  delete module;
  return 0;
}