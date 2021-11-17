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
  Type *FloatType = Type::get_float_type(module);  

  // main函数
  auto mainFun = Function::create(FunctionType::get(Int32Type, {}),
                                  "main", module);
  auto bb = BasicBlock::create(module, "entry", mainFun);
  // BasicBlock的名字在生成中无所谓,但是可以方便阅读
  builder->set_insert_point(bb);


  auto a = builder->create_alloca(FloatType);// alloca float
  builder->create_store(CONST_FP(5.555), a);//store a with 5.55
  auto load0 = builder->create_load(a);//load a to load0
  auto fcmp = builder->create_fcmp_gt(load0, CONST_FP(1.0));  // b和a的比较

  auto trueBB = BasicBlock::create(module, "trueBB", mainFun);    // true分支
  auto falseBB = BasicBlock::create(module, "falseBB", mainFun);  // false分支

  auto br = builder->create_cond_br(fcmp, trueBB, falseBB);  // 条件BR
  DEBUG_OUTPUT // 我调试的时候故意留下来的,以醒目地提醒你这个调试用的宏定义方法
  builder->set_insert_point(trueBB);  // if true; 分支的开始需要SetInsertPoint设置
  auto a1 = builder->create_alloca(FloatType);// alloca float
  builder->create_store(CONST_INT(233), a1);//store a1 with 233
  auto load1 = builder->create_load(a1);//load a1 to load1
  builder->create_ret(load1);//ret load1
  
  builder->set_insert_point(falseBB);  // if false
  auto a2 = builder->create_alloca(FloatType);// alloca float
  builder->create_store(CONST_INT(0), a2);//store a2 with 0
  auto load2 = builder->create_load(a2);//load a2 to load2
  builder->create_ret(load2);//ret load2

  std::cout << module->print();
  delete module;
  return 0;
}