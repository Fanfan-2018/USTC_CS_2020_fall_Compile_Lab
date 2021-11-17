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
  //main函数
  auto mainFun = Function::create(FunctionType::get(Int32Type, {}),
                                  "main", module);
  auto bb = BasicBlock::create(module, "entry", mainFun);
  // BasicBlock的名字在生成中无所谓,但是可以方便阅读
  builder->set_insert_point(bb);  

  auto a = builder->create_alloca(Int32Type);         //给a分配内存空间
  auto i = builder->create_alloca(Int32Type);         //给i分配内存空间 
  builder->create_store(CONST_INT(10), a);   //a = 10
  builder->create_store(CONST_INT(0), i);    //i = 0  
  auto load0 = builder->create_load(a);//load a to load0
  auto load1 = builder->create_load(i);//load i to load1
  auto icmp0 = builder->create_icmp_lt(load1, CONST_INT(10));   // i < 10

  auto trueBB0 = BasicBlock::create(module, "trueBB0", mainFun);    // true分支
  auto falseBB0 = BasicBlock::create(module, "falseBB0", mainFun);  // false分支

  auto br0 = builder->create_cond_br(icmp0, trueBB0, falseBB0);  // 条件BR

  DEBUG_OUTPUT // 我调试的时候故意留下来的,以醒目地提醒你这个调试用的宏定义方法
  builder->set_insert_point(trueBB0);  // if true; 分支的开始需要SetInsertPoint设置
  auto load2 = builder->create_load(i);//load i to load2
  auto load3 = builder->create_load(a);//load a to load3
  auto add0 = builder->create_iadd(load2, CONST_INT(1));    // i = i + 1
  auto add1 = builder->create_iadd(load3, load2);            // a = a + i    
  builder->create_store(add1, a); //store a[1] with the result of mul
  builder->create_store(add0, i); //store a[1] with the result of mul
  auto icmp1 = builder->create_icmp_lt(add0, CONST_INT(10));   // i < 10
  auto br1 = builder->create_cond_br(icmp1, trueBB0, falseBB0);  // 条件BR

  builder->set_insert_point(falseBB0);  // if false
  auto load4 = builder->create_load(a);//load a to load4
  builder->create_ret(load4);
  std::cout << module->print();
  delete module;
  return 0;
}