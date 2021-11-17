# lab3 实验报告

## PB18000006 范翔宇

## 问题1: cpp与.ll的对应

请描述你的cpp代码片段和.ll的每个BasicBlock的对应关系。描述中请附上两者代码。

### assign_hand.ll和assign_generator.cpp的基本块的对应关系

assign_hand.ll的基本块只有一个:

```ll
entry:
  %0 = alloca [10 x i32]
  %1 = getelementptr [10 x i32], [10 x i32]* %0, i32 0, i32 0
  store i32 10, i32* %1
  %2 = getelementptr [10 x i32], [10 x i32]* %0, i32 0, i32 1
  %3 = load i32, i32* %1
  %4 = mul i32 %3, 2
  store i32 %4, i32* %2
  %5 = load i32, i32* %2
  ret i32 %5
```

在assign_generator.cpp中与上述基本块对应的代码部分如下:

```cpp
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
```

一一对应，下同

### fun_hand.ll和fun_generator.cpp的基本块的对应关系:

有两个基本块

在fun_hand.ll中第一个基本块的对应callee函数:

```
entry:
  %1 = mul i32 %0, 2
  ret i32 %1
```

在fun_generator.cpp中与上述基本块对应的代码部分如下:

```cpp
builder->set_insert_point(bb);                        // 一个BB的开始,将当前插入指令点的位置设在bb
  
  
  std::vector<Value *> args;  // 获取callee函数的形参,通过Function中的iterator
  for (auto arg = calleeFun->arg_begin(); arg != calleeFun->arg_end(); arg++) {
    args.push_back(*arg);   // * 号运算符是从迭代器中取出迭代器当前指向的元素
  }
  
  auto mul = builder->create_imul(args[0], CONST_INT(2));  // mul = a * 2,the result   
  builder->create_ret(mul); //ret mul
```

在fun_hand.ll中第二个基本块对应main函数

```
entry:
  %0 = call i32 @callee(i32 110)
  ret i32 %0
```

在fun_generator.cpp中与上述基本块对应的代码部分如下:

```cpp
builder->set_insert_point(bb);

  auto call = builder->create_call(calleeFun, {CONST_INT(110)});  
  builder->create_ret(call);
```

### if_hand.ll和if_generator.cpp的基本块的对应关系:

有三个基本块，分别对应main trueBB falseBB

在if_hand.ll中第一个基本块为:

```
entry:
  %0 = alloca float
  store float 0x40163851e0000000, float* %0
  %1 = load float, float* %0
  %2 = fcmp ugt float %1,0x3ff0000000000000
  br i1 %2, label %trueBB, label %falseBB
```

在if_generator.cpp中与上述基本块对应的代码部分如下:

```cpp
builder->set_insert_point(bb);

  auto a = builder->create_alloca(FloatType);// alloca float
  builder->create_store(CONST_FP(5.555), a);//store a with 5.55
  auto load0 = builder->create_load(a);//load a to load0
  auto fcmp = builder->create_fcmp_gt(load0, CONST_FP(1.0));  // b和a的比较

  auto trueBB = BasicBlock::create(module, "trueBB", mainFun);    // true分支
  auto falseBB = BasicBlock::create(module, "falseBB", mainFun);  // false分支

  auto br = builder->create_cond_br(fcmp, trueBB, falseBB);  // 条件BR
```

在if_hand.ll中第二个基本块为:

```
trueBB:
  %3 = alloca float
  store i32 233, float* %3
  %4 = load float, float* %3
  ret float %4
```

在if_generator.cpp中与上述基本块对应的代码部分如下:

```cpp
 builder->set_insert_point(trueBB);  // if true; 分支的开始需要SetInsertPoint设置
  auto a1 = builder->create_alloca(FloatType);// alloca float
  builder->create_store(CONST_INT(233), a1);//store a1 with 233
  auto load1 = builder->create_load(a1);//load a1 to load1
  builder->create_ret(load1);//ret load1
```

在if_hand.ll中第三个基本块为 :

```
falseBB:
  %5 = alloca float
  store i32 0, float* %5
  %6 = load float, float* %5
  ret float %6
```

在if_generator.cpp中与上述基本块对应的代码部分如下:

```cpp
builder->set_insert_point(falseBB);  // if false
  auto a2 = builder->create_alloca(FloatType);// alloca float
  builder->create_store(CONST_INT(0), a2);//store a2 with 0
  auto load2 = builder->create_load(a2);//load a2 to load2
  builder->create_ret(load2);//ret load2
```

### while_hand.ll和while_generator.cpp的基本块的对应关系:

有三个基本块，为mian trueBB0 falseBB0

在while_hand.ll中第一个基本块为:

```
entry:
  %0 = alloca i32
  %1 = alloca i32
  store i32 10, i32* %0
  store i32 0, i32* %1
  %2 = load i32, i32* %0
  %3 = load i32, i32* %1
  %4 = icmp slt i32 %3, 10
  br i1 %4, label %trueBB0, label %falseBB0
```

在while_generator.cpp中与上述基本块对应的代码部分如下:

```cpp
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
```

在while_hand.ll中第二个基本块为:

```
trueBB0:
  %5 = load i32, i32* %1
  %6 = load i32, i32* %0
  %7 = add i32 %5, 1
  %8 = add i32 %6, %5
  store i32 %8, i32* %0
  store i32 %7, i32* %1
  %9 = icmp slt i32 %7, 10
  br i1 %9, label %trueBB0, label %falseBB0
```

在while_generator.cpp中与上述基本块对应的代码部分如下:

```cpp
builder->set_insert_point(trueBB0);  // if true; 分支的开始需要SetInsertPoint设置
  auto load2 = builder->create_load(i);//load i to load2
  auto load3 = builder->create_load(a);//load a to load3
  auto add0 = builder->create_iadd(load2, CONST_INT(1));    // i = i + 1
  auto add1 = builder->create_iadd(load3, load2);            // a = a + i    
  builder->create_store(add1, a); //store a[1] with the result of mul
  builder->create_store(add0, i); //store a[1] with the result of mul
  auto icmp1 = builder->create_icmp_lt(add0, CONST_INT(10));   // i < 10
  auto br1 = builder->create_cond_br(icmp1, trueBB0, falseBB0);  // 条件BR
```

在while_hand.ll中第三个基本块为:

```
falseBB0:
  %10 = load i32, i32* %0
  ret i32 %10
```

在while_generator.cpp中与上述基本块对应的代码部分如下:

```cpp
builder->set_insert_point(falseBB0);  // if false
  auto load4 = builder->create_load(a);//load a to load4
  builder->create_ret(load4);
```

### Little Sum

对于ll文件来说一个基本块的开始应该是由这一块的label开始的

而对于cpp文件来说，基本块的开始应该由``` builder->set_insert_point(bb); ```决定

下面是一些对应关系，ll文件语句在上方，cpp文件语句在下方

先是数组的一些操作

定义数组：

```
  %0 = alloca [10 x i32]
```

```cpp
  auto *ArrayType = ArrayType::get(Int32Type, 10);//前提工作
  auto initializer = ConstantZero::get(Int32Type, module);//前提工作
  auto a = builder->create_alloca(ArrayType);   // a[10]的存放
```

接着是定义int float型

float：

```
  %0 = alloca float
```

```cpp
  auto a = builder->create_alloca(FloatType);// alloca float
```

int：

```
  %0 = alloca i32
```

```cpp
  auto a = builder->create_alloca(Int32Type);         //给a分配内存空间
```

数组对应的元素

```
  %1 = getelementptr [10 x i32], [10 x i32]* %0, i32 0, i32 0
```

```cpp
  auto a0GEP = builder->create_gep(a, {CONST_INT(0), CONST_INT(0)});  // a[0]
```

接着是load和store操作

load:

```
  %2 = load i32, i32* %0
```

```cpp
  auto load0 = builder->create_load(a);//load a to load0
```

store:

```
  store i32 10, i32* %0
```

```cpp
  builder->create_store(CONST_INT(10), a);   //a = 10
```

后面是ret操作

ret：

```
  ret i32 %10
```

```cpp
  builder->create_ret(load4);
```

然后是比较分支操作

cmp:

```
  %4 = icmp slt i32 %3, 10
```

```cpp
  auto icmp0 = builder->create_icmp_lt(load1, CONST_INT(10));   // i < 10
```

br:

```
  br i1 %4, label %trueBB0, label %falseBB0
```

```cpp
  auto br0 = builder->create_cond_br(icmp0, trueBB0, falseBB0);  // 条件BR
```

暂时就这么多

## 问题2: Visitor Pattern

请指出visitor.cpp中，`treeVisitor.visit(exprRoot)`执行时，以下几个Node的遍历序列:numberA、numberB、exprC、exprD、exprE、numberF、exprRoot。 

序列请按如下格式指明： 

exprRoot->numberF->exprE->numberA->exprD

我的答案：exprRoot->exprE->exprC->numberA->numberB->exprD->numberB->numberA->numberF

## 问题3: getelementptr

请给出`IR.md`中提到的两种getelementptr用法的区别,并稍加解释:

 \- `%2 = getelementptr [10 x i32], [10 x i32]* %1, i32 0, i32 %0` 

 \- `%2 = getelementptr i32, i32* %1 i32 %0` 

上面这种用于具体大小的数组类型的元素的地址，第一个参数是计算基础类型，第二第三个参数表示索引开始的指针类型及指针，第四个参数:```i32 0```表示偏移量，并且其偏移量的单位为```10*i32```也就是40个字节，第五个参数:```i32 %0```也表示偏移量，但是偏移单位是```i32```也就是4个字节。

而下面这种用于获取无具体大小的数组类型的元素的地址，例如gcd中的u[]。第四个参数:```i32 %0```表示偏移量，并且偏移单位为```i32```也就是4个字节。

## 实验难点

描述在实验中遇到的问题、分析和解决方案

刚开始写实验的时候不熟悉ll语法和cpp语法，即使照葫芦画瓢也花费了不少功夫，但后来看多了就有感觉了，逐渐到了c与ll与cpp的关系，也明白了三者之间的转换方法等。在编写cpp文件的时候，一开始不了解具体格式，不清楚哪些是需要的哪些是非必需的，常发生画蛇添足和缺斤短两，后面一一调试，也算顺利。最后回答问题的时候，需要思考好久，认真斟酌才能定夺。

## 实验反馈

每次实验确实是一次体验很好的实验，唯一不足的是我记错了ddl时间，啊。。。。。。。。