# Lab5 实验报告-阶段一



小组成员 姓名 学号

队长	李达天	PB18000294

队员	范翔宇	PB18000006

## 实验要求

阅读代码，理清内在逻辑，了解优化的基本流程，掌握开发基于LightIR的优化Pass

## 思考题

### LoopSearch

1. 在每个强连通图(集合)中，找到前驱为end的结点，即为循环入口。如果没有找到，则在reserved中找最后一个不等于end的后继结点。一般来说，IR中的循环条件块只有一个，而且在LoopSearch.cpp文件中，对每个循环也只调用了一次find_loop_base函数，而find_loop_base函数只有一个返回值，因此不会超过1。
2. 从外到内找每个强连通图，对于每个强连通图都通过find_loop_base寻找入口，并且利用func2loop、base2loop、loop2base、bb2base等数组或函数，将循环与函数、基本块与循环以及基本块与块所在最内层循环的入口一一对应，然后删除最外层循环入口结点，进一步处理内循环，因为删掉最外层入口循环入口之后，生成强连通分量便是内循环的了，就这样一层层循环去掉，就解决了循环嵌套的问题。



## Mem2reg

1. 如果n支配m的前驱结点，但是不严格支配m，则称m为n的支配边界，其中m通常为分支汇合点。

2. 例如：

   ```
   a = 1;
   if (v < 10)
       a = 2;
   b = a;
   ```

   由于静态单赋值只能让变量赋值一次，那么这里的b如何取值是无法决定的，而Phi 就是解决这样的问题，引入Phi后有：

   ```
   a1 = 1;
   if (v < 10)
       a2 = 2;
   b = PHI(a1, a2);
   ```

   那么Phi则会根据到达的是a1还是a2而选择给b赋何值。Phi就是形如PHI(a1，a2)的函数。对于CFG的每一个前驱块，Phi函数都应该有一个参数与之对应，在这里起选择作用。

3. 删去了繁冗的alloca、load、store操作，使原本存储在存储器的数现在只在寄存器中短暂存在，而且如果某个赋值是无法确定的，即可能来自两个甚至多个分支，则调用Phi函数处理，而不是直接赋值。

4. 遍历支配树中的支配边界，判断如果没有Phi，即find=end，则create 一个phi，将var置为左值，并在该支配边界对应bb块的指令开头加上phi，将当前bb块压入work_list，将bb_has_var_phi置为true，即利用支配树的支配边界及其一些变量完成了phi节点的放置。

5. 通过维护一个栈来维护变量值，具体参见如下代码，在栈中通过l_val寻找之前压栈的值，用其代替load指令。维护该栈的方式就是当有store指令时，将r_val压进栈作为lval的最新定义。

   ```cpp
   //代替load指令
   auto l_val = static_cast<LoadInst *>(instr)->get_lval();
   
   if (!IS_GLOBAL_VARIABLE(l_val) && !IS_GEP_INSTR(l_val))
   {
   	if ( var_val_stack.find(l_val)!=var_val_stack.end())
       {
       	instr->replace_all_use_with(var_val_stack[l_val].back());
           wait_delete.push_back(instr);
       }
   }
   ```

   ```c++
   //store 栈的维护
   auto l_val = static_cast<StoreInst *>(instr)->get_lval();
   auto r_val = static_cast<StoreInst *>(instr)->get_rval();
   if (!IS_GLOBAL_VARIABLE(l_val) && !IS_GEP_INSTR(l_val))
   {
       var_val_stack[l_val].push_back(r_val);
       wait_delete.push_back(instr);    
   }
   ```

   

### 代码阅读总结

#### LoopSearch

首先根据每个基本块建立CFG图，当dump为true时打印该图；而后由外到内找到强连通图，对于每个强连通图找到循环入口结点，用func记录函数与循环的对应关系，base2loop和loop2base记录base块与循环的关系，bb2base记录循环中的一个块到这个块所在循环的（最内层循环）的关系。最后删除最外层循环入口结点，即去掉最外层循环，生成内循环的强连通分量，进一步处理内循环。

#### Mem2reg

doms记录支配该点的结点，idom记录离该点最近的支配点，dom__frontier记录支配边界点，succ_block记录支配树中的后继块。依次创建后序，创建idom，创建支配边界，创建后继，即可成功创建后序支配树。然后对于每个函数，若其块的大小大于等于1，则创建$\phi$函数，并进行重命名操作（具体参见前面例子中的a变成a1和a2），并删去冗余的alloca、load、store操作。删除指令是通过维护栈完成的，当有store指令时，将r_val压进栈作为lval的最新定义，当遇到对应的load指令时，在栈中通过l_val寻找之前压栈的值，用其代替load指令。

### 实验反馈 （可选 不会评分）



对本次实验的建议



### 组间交流 （可选）



本次实验和哪些组（记录组长学号）交流了哪一部分信息