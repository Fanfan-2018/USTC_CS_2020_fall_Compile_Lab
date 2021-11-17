#include "cminusf_builder.hpp"
#include<iostream>
#include<memory>

// use these macros to get constant value
#define CONST_FP(num) \
    ConstantFP::get((float)num, module.get())
#define CONST_ZERO(type) \
    ConstantZero::get(var_type, module.get())


// You can define global variables here
// to store state

/*
 * use CMinusfBuilder::Scope to construct scopes
 * scope.enter: enter a new scope
 * scope.exit: exit current scope
 * scope.push: add a new binding to current scope
 * scope.find: find and return the value bound to the name
 */
#define CONST_INT(num) \
    ConstantInt::get(num, module1)

#define CONST_FP(num) \
    ConstantFP::get(num, module1) // 得到常数值的表示,方便后面多次用到
//llvm
auto module1 = new Module("cminusf");
auto builder = new IRBuilder(nullptr, module1);//add

//Type constant
Type* TyVoid = Type::get_void_type(module1);//void
Type* TyInt32 = Type::get_int32_type(module1);//int
Type* TyFloat = Type::get_float_type(module1);//float

PointerType* TyInt32Ptr = PointerType::get(TyInt32);//int*
PointerType* TyFloatPtr = PointerType::get(TyFloat);//float*

//global variables
Value* value;//expression value
Value* varAlloca;//for store value or get pointer of array
Type* paramType;//for function declaration
std::string paramId;
Scope scope;//symbol table
Scope gscope;//global symbol table
Function* fun;//current function for basicblock creation
bool hasReturn;//make sure if has nextBB
unsigned num;//for array declaration
bool Global;//judge whether global

void CminusfBuilder::visit(ASTProgram &node) {
    for(auto decl: node.declarations){//call visit
        decl->accept(*this);
    }
 }

void CminusfBuilder::visit(ASTNum &node) {
    varAlloca = nullptr;
    //判断是int还是float
    if(node.type==TYPE_INT)   {num = node.i_val; value = CONST_INT(node.i_val);}
    if(node.type==TYPE_FLOAT) {num = node.f_val;value = CONST_FP(node.f_val); }
}

void CminusfBuilder::visit(ASTVarDeclaration &node) { 
    auto type = TyInt32;
    if(node.type==TYPE_INT)   type = TyInt32; // int a
    else if(node.type==TYPE_FLOAT)  type = TyFloat;//float a
    else  type = TyVoid;
    if(node.num != nullptr){//isarray
        node.num->accept(*this);
        type = ArrayType::get(type, num);
    }
    //判断是否为全局变量
    if(scope.in_global()){
        //是则存入gscope
        auto zeroInitializer = ConstantZero::get(type,module.get());
        GlobalVariable *alloca = GlobalVariable::create(node.id,module.get(),type,false,zeroInitializer);
        gscope.enter();
        gscope.push(node.id,alloca);
    }else{
        //否则存入scope
        auto alloca = builder->create_alloca(type);
        scope.push(node.id, alloca);
    }
}

void CminusfBuilder::visit(ASTFunDeclaration &node) { 
    std::vector<Type *> paramTypes;
    std::vector<std::string> paramIds; // define param attributes

    for(auto param : node.params){
        param->accept(*this); // visit param node
        paramTypes.push_back(paramType);
        paramIds.push_back(paramId);
    }
    auto funtype = TyVoid;
    //判断函数类型
    if(node.type==TYPE_INT)  funtype=TyInt32;
    else if(node.type==TYPE_FLOAT) funtype=TyFloat;
    fun = Function::create(
        FunctionType::get(funtype, paramTypes),
        node.id,
        module.get()
    );//create function in llvm

    scope.push(node.id, fun);

    std::vector<Value *> args;  
    for (auto arg = fun->arg_begin(); arg != fun->arg_end(); arg++) {
        args.push_back(*arg);
    }//get args in llvm

    auto bb = BasicBlock::create(module1, "", fun);
    builder->set_insert_point(bb);

    scope.enter();//create new symbol table inheriting global value
    for (int i=0;i<args.size();i++){
        auto alloca = builder->create_alloca(paramTypes[i]); //create allocation for params
        builder->create_store(args[i], alloca); //store value to alloca
        scope.push(paramIds[i], alloca); //put into symbol table
    }

    node.compound_stmt->accept(*this);
    
    scope.exit();//distory local declaration in function
}

void CminusfBuilder::visit(ASTParam &node) { 
    paramId = node.id; //let fun_decl know param attribute name
    if(node.isarray){// let fun_decl know param attribute type
        //判断array类型
        if(node.type == TYPE_INT){
            paramType = TyInt32Ptr;
        }else if(node.type == TYPE_FLOAT){
            paramType = TyFloatPtr;
        }else{
            paramType = TyVoid;
        }

    }
    //判断类型
    else if(node.type == TYPE_INT){
        paramType = TyInt32;
    }
    else if(node.type == TYPE_FLOAT){
        paramType = TyFloat;
    }
    else{
        paramType = TyVoid;
    }
}

void CminusfBuilder::visit(ASTCompoundStmt &node) { 
    gscope.enter();//tag g
    scope.enter();//new symbol table
    for(auto decl : node.local_declarations){
        decl->accept(*this);
    }
    for(auto decl : node.statement_list){
        decl->accept(*this);
    }
    scope.exit();//destory local declarations
    gscope.exit();//tag g
}

void CminusfBuilder::visit(ASTExpressionStmt &node) {
    node.expression->accept(*this);
 }

void CminusfBuilder::visit(ASTSelectionStmt &node) {
    node.expression->accept(*this);
    Value* cmp;
    auto selty = value->get_type();
    //判断类型，并跟0进行比较
    if(selty->is_integer_type() == 1){
        cmp = builder->create_icmp_ne(value, ConstantInt::get(0,module.get()));//expr not equal 0 goto trueBB
    }else{
        auto zero=CONST_FP(0);
        cmp = builder->create_fcmp_ne(value, zero);
    }
    auto trueBB = BasicBlock::create(module1, "", fun);
    BasicBlock* nextBB;
    //如果没有else块
    //不能接收nextBB为空的情形
    if(node.else_statement == nullptr){// no else
        nextBB = BasicBlock::create(module1, "", fun);
        builder->create_cond_br(cmp, trueBB, nextBB);
        hasReturn = false;
        builder->set_insert_point(trueBB);
        node.if_statement->accept(*this);
        if(!hasReturn){
            builder->create_br(nextBB);
        }
    }
    //有else块
    else{
        auto falseBB = BasicBlock::create(module1, "", fun);
        builder->create_cond_br(cmp, trueBB, falseBB);
        nextBB = nullptr;
        hasReturn = false;
        builder->set_insert_point(trueBB);
        node.if_statement->accept(*this);
        if(!hasReturn){// return statement not in if(){}
            nextBB = BasicBlock::create(module1, "", fun);
            builder->create_br(nextBB);
        }

        hasReturn = false;
        builder->set_insert_point(falseBB);
        node.else_statement->accept(*this);
        if(!hasReturn){//return statement not in else(){}
            if(nextBB == nullptr)nextBB = BasicBlock::create(module1, "", fun);
            builder->create_br(nextBB);
        }
    }
    if(nextBB != nullptr){
        builder->set_insert_point(nextBB);
    }    
 }




void CminusfBuilder::visit(ASTIterationStmt &node) {
    node.expression->accept(*this);
    auto judgeBB = BasicBlock::create(module1, "", fun);
    auto continueBB = BasicBlock::create(module1, "", fun);
    auto breakBB = BasicBlock::create(module1, "", fun);
    auto iterty = value->get_type();
    Value* cmp;
    builder->create_br(judgeBB); // last bb end with jump to judegeBB
    builder->set_insert_point(judgeBB);

    node.expression->accept(*this);
    //判断类型，并且跟0比较
    if(iterty->is_integer_type() == 1){
        auto zero = CONST_INT(0);
        cmp = builder->create_icmp_ne(value, zero);
    }else{
        auto zero = CONST_FP(0);
        cmp = builder->create_fcmp_ne(value, zero);
    }
    auto br = builder->create_cond_br(cmp, continueBB, breakBB);
    hasReturn = false;
    builder->set_insert_point(continueBB);
    node.statement->accept(*this);
    if(!hasReturn){
        br = builder->create_br(judgeBB);
    }
    if(breakBB!=nullptr){
        builder->set_insert_point(breakBB);    
    }
 }

void CminusfBuilder::visit(ASTReturnStmt &node) {
     if(node.expression == nullptr){//void fun
        builder->create_void_ret();
    }
    else{
        //return的时候，要注意类型转换
        node.expression->accept(*this);
        if(fun->get_return_type()->is_integer_type() == 1 && value->get_type()->is_float_type() == 1){
            builder->create_ret(builder->create_fptosi(value,TyInt32));
        }else if(fun->get_return_type()->is_float_type() == 1 && value->get_type()->is_integer_type() == 1){
            builder->create_ret(builder->create_sitofp(value,TyFloat));
        }else{
            builder->create_ret(value);
        }
    }
    hasReturn = true;
 }

void CminusfBuilder::visit(ASTVar &node) {
    Global = 0;
    //先在局部变量里面找
    auto alloca = scope.find(node.id);//find its declaration
    //如果在局部变量里面没找到，再找全局变量
    if(alloca == nullptr){
        gscope.enter();
        alloca = gscope.find(node.id);
        Global = 1;
        gscope.exit();
    }
    if(node.expression != nullptr){//isarray
        std::vector<Value *> values;//
        node.expression->accept(*this);
        auto arrayIndex = value;
        
        //check negative index
        //检查错误
        auto exception = scope.find("neg_idx_except");
        auto exceptBB = BasicBlock::create(module1, "", fun);
        auto nextBB = BasicBlock::create(module1, "", fun);

        auto icmp = builder->create_icmp_ge(arrayIndex, CONST_INT(0));
        builder->create_cond_br(icmp, nextBB, exceptBB);

        builder->set_insert_point(exceptBB);
        builder->create_call(exception,values);
        builder->create_br(nextBB);

        builder->set_insert_point(nextBB);
        value = builder->create_load(alloca);
        if(value->get_type()  ==TyInt32Ptr || value->get_type() == TyFloatPtr){//array is parameter of function

            alloca = builder->create_gep(value, std::vector<Value *>({arrayIndex}));
        }
        else{//array declare in this function or isglobal
            alloca = builder->create_gep(alloca, std::vector<Value *>({CONST_INT(0), arrayIndex}));
        }
    }
    //赋值
    varAlloca = alloca;
    value = builder->create_load(varAlloca);
}

void CminusfBuilder::visit(ASTAssignExpression &node) {
    node.var->accept(*this);
    //这里的Global1和Global2是为了处理以下这种情况
    //a op b，a是全局变量，b是局部变量类似这种
    int Global1 = Global;
    auto storePtr = (AllocaInst *)varAlloca;
    int Global2 = Global;
    node.expression->accept(*this);
    if(Global1 == 0&&Global2==0){//如果都不是全局变量，那么我就直接类型转换就可以了
        if(storePtr->get_alloca_type()->is_integer_type() == 1 && value->get_type()->is_float_type() == 1){
            //float -> int
            builder->create_store(builder->create_fptosi(value,TyInt32),storePtr);
        }else if(storePtr->get_alloca_type()->is_float_type() == 1 && value->get_type()->is_integer_type() == 1){
            //int -> float
            builder->create_store(builder->create_sitofp(value,TyFloat),storePtr);
        }
        else{
            //simple store
            builder->create_store(value,storePtr);        
        }    
    }else{//如果有全局变量
            //这里load是因为全局变量情况下只能得到pointer type
            auto a = builder->create_load(storePtr);
            if(a->get_type()->is_integer_type() == 1 && value->get_type()->is_float_type() == 1){
                builder->create_store(builder->create_fptosi(value,TyInt32),storePtr);
            }else if(a->get_type()->is_float_type() == 1 && value->get_type()->is_integer_type() == 1){
                builder->create_store(builder->create_sitofp(value,TyFloat),storePtr);
            }else{
                builder->create_store(value,storePtr);
            }
    }
 }


void CminusfBuilder::visit(ASTSimpleExpression &node) {
     if(node.additive_expression_r == nullptr){//只有左表达式
        node.additive_expression_l->accept(*this);
        return;
    }
    node.additive_expression_l->accept(*this);
    auto type1 = value->get_type();
    auto leftalloca=builder->create_alloca(TyFloat);
    //我们决定使用fcmp，所以要注意类型转换
    if(type1->is_integer_type()){
        builder->create_store(builder->create_sitofp(value,TyFloat),leftalloca);
    }else{
        builder->create_store(value,leftalloca);
    }
    auto leftvalue = builder->create_load(leftalloca);
    node.additive_expression_r->accept(*this);
    auto type2 = value->get_type();
    auto rightalloca=builder->create_alloca(TyFloat);
    //同理
    if(type2->is_integer_type()){
        builder->create_store(builder->create_sitofp(value,TyFloat),rightalloca);
    }else{
        builder->create_store(value,rightalloca);
    }
    auto rightvalue = builder->create_load(rightalloca);
    switch(node.op){  //全部用fcmp
    case(OP_LE)://<=
        value = builder->create_fcmp_le(leftvalue,rightvalue);
        break;
    case(OP_LT)://<
        value = builder->create_fcmp_lt(leftvalue,rightvalue);
        break;
    case(OP_GT)://>
        value = builder->create_fcmp_gt(leftvalue,rightvalue);
        break;
    case(OP_GE)://>=
        value = builder->create_fcmp_ge(leftvalue,rightvalue);
        break;
    case(OP_EQ)://==
        value = builder->create_fcmp_eq(leftvalue,rightvalue);
        break;
    case(OP_NEQ)://!=
        value = builder->create_fcmp_ne(leftvalue,rightvalue);
        break;
    }
    value = builder->create_zext(value,TyInt32);//这里我把判断之后的结果，由i1类型转换成i32类型了，因为认为i1和i32都是int类型，所以后续没有函数可以判断
    //zext
}

void CminusfBuilder::visit(ASTAdditiveExpression &node) { 
    if(node.additive_expression == nullptr){
        node.term->accept(*this);
        return;
    }

    node.additive_expression->accept(*this);
    auto leftValue = value;
    node.term->accept(*this);
    auto rightValue = value;
    switch(node.op){
    //这里加减要注意类型转换！
    case(OP_PLUS)://+
        if(leftValue->get_type()->is_integer_type() == 1 && rightValue->get_type()->is_integer_type() == 1){
            value = builder->create_iadd(leftValue, rightValue);
        }else if(leftValue->get_type()->is_float_type() == 1 && rightValue->get_type()->is_integer_type() == 1){
            value = builder->create_fadd(leftValue, builder->create_sitofp(rightValue,TyFloat));        
        }else if(leftValue->get_type()->is_integer_type() == 1 && rightValue->get_type()->is_float_type() == 1){
            value = builder->create_fadd(builder->create_sitofp(leftValue,TyFloat), rightValue);        
        }else if(leftValue->get_type()->is_float_type() == 1 && rightValue->get_type()->is_float_type() == 1){
            value = builder->create_fadd(leftValue, rightValue);        
        }
        break;
    case(OP_MINUS)://-
        if(leftValue->get_type()->is_integer_type() == 1 && rightValue->get_type()->is_integer_type() == 1){
            value = builder->create_isub(leftValue, rightValue);
        }else if(leftValue->get_type()->is_float_type() == 1 && rightValue->get_type()->is_integer_type() == 1){
            value = builder->create_fsub(leftValue, builder->create_sitofp(rightValue,TyFloat));        
        }else if(leftValue->get_type()->is_integer_type() == 1 && rightValue->get_type()->is_float_type() == 1){
            value = builder->create_fsub(builder->create_sitofp(leftValue,TyFloat), rightValue);        
        }else if(leftValue->get_type()->is_float_type() == 1 && rightValue->get_type()->is_float_type() == 1){
            value = builder->create_fsub(leftValue, rightValue);           
        }
        break; 
    
    }
}

void CminusfBuilder::visit(ASTTerm &node) { 
    if(node.term == nullptr){
        node.factor->accept(*this);
        return;
    }

    node.term->accept(*this);
    auto leftValue = value;

    node.factor->accept(*this);
    auto rightValue = value;
    //同样需要注意类型转换
    switch(node.op){
    case(OP_MUL)://*
        if(leftValue->get_type()->is_integer_type() == 1 && rightValue->get_type()->is_integer_type() == 1){
            value = builder->create_imul(leftValue, rightValue);
        }else if(leftValue->get_type()->is_float_type() == 1 && rightValue->get_type()->is_integer_type() == 1){
            value = builder->create_fmul(leftValue, builder->create_sitofp(rightValue,TyFloat));        
        }else if(leftValue->get_type()->is_integer_type() == 1 && rightValue->get_type()->is_float_type() == 1){
            value = builder->create_fmul(builder->create_sitofp(leftValue,TyFloat), rightValue);        
        }else if(leftValue->get_type()->is_float_type() == 1 && rightValue->get_type()->is_float_type() == 1){
            value = builder->create_fmul(leftValue, rightValue);            
        }
	    break;
	   
    case(OP_DIV):///
        if(leftValue->get_type()->is_integer_type() == 1 && rightValue->get_type()->is_integer_type() == 1){
            value = builder->create_isdiv(leftValue, rightValue);
        }else if(leftValue->get_type()->is_float_type() == 1 && rightValue->get_type()->is_integer_type() == 1){
            value = builder->create_fdiv(leftValue, builder->create_sitofp(rightValue,TyFloat));        
        }else if(leftValue->get_type()->is_integer_type() == 1 && rightValue->get_type()->is_float_type() == 1){
            value = builder->create_fdiv(builder->create_sitofp(leftValue,TyFloat), rightValue);        
        }else if(leftValue->get_type()->is_float_type() == 1 && rightValue->get_type()->is_float_type() == 1){
            value = builder->create_fdiv(leftValue, rightValue);     
        }
	    break;
    }

}

void CminusfBuilder::visit(ASTCall &node) {
    auto calleeFun = scope.find(node.id);
    if(calleeFun == nullptr){
        gscope.enter();
        calleeFun = gscope.find(node.id);
        gscope.exit();
    }
    Function *calleefun = (Function *)calleeFun;
    FunctionType *callee = calleefun->get_function_type();
    std::vector<Value *> values;//parameter passed to call
    int i = 0;
    for(auto arg : node.args){
        arg->accept(*this);
        Type *parTy = callee->get_param_type(i);
        //判断参数类型
        if(value->get_type()->is_integer_type()==1 && parTy->is_float_type()==1){
            value = builder->create_sitofp(value,TyFloat);
        }
        else if(value->get_type()->is_float_type()==1 && parTy->is_integer_type()==1){
            value = builder->create_fptosi(value,TyInt32);
        }
        i++;
        if(value->get_type()->is_integer_type() != 1 && value->get_type()->is_pointer_type() != 1 && value->get_type()->is_float_type() != 1 && varAlloca != nullptr){//pointer ptr???
            value = builder->create_gep(varAlloca, {CONST_INT(0), CONST_INT(0)});
        }// pass array to function
        values.push_back(value);
    }
    value = builder->create_call(calleeFun, values);
 //500行hhhh
 }
