#include "ConstPropagation.hpp"
#include "logging.hpp"
#include "LoopSearch.hpp"
#include <iostream>
#define IS_GLOBAL_VARIABLE(l_val) dynamic_cast<GlobalVariable *>(l_val)
#define IS_GEP_INSTR(l_val) dynamic_cast<GetElementPtrInst *>(l_val)
#define CONST_FP(num) \
    ConstantFP::get((float)num, this->m_)
#define CONST_INT(num) \
    ConstantInt::get(num, this->m_)
#define IS_C0NST(num) \
    bool (cast_constantint(num)||cast_constantfp(num))
int flag;
std::map<std::string, std::vector<BasicBlock *>> judegstack;//存储while或者if的判断块
std::map<std::string, std::vector<BasicBlock *>> whilestack;//存储while循环块
std::map<Value*, std::vector<Instruction *>> phistack;//存储phi
std::map<Value*, std::vector<Value *>> numstack;//存储
std::map<std::string, std::vector<BasicBlock *>> deletestack;
//存储phi中，label和num的关系，然后我后面我要删除这个label，就等于另一个label的值
std::vector<Instruction *> wait_delete;
// 给出了返回整形值的常数折叠实现，大家可以参考，在此基础上拓展
// 当然如果同学们有更好的方式，不强求使用下面这种方式
int count;
ConstantInt *ConstIntFolder::computeint(
    Instruction::OpID op,
    ConstantInt *value1,
    ConstantInt *value2)
{

    int c_value1 = value1->get_value();
    int c_value2 = value2->get_value();
    switch (op)
    {
    case Instruction::add:
        return ConstantInt::get(c_value1 + c_value2, module_);
        break;
    case Instruction::sub:
        return ConstantInt::get(c_value1 - c_value2, module_);
        break;
    case Instruction::mul:
        return ConstantInt::get(c_value1 * c_value2, module_);
        break;
    case Instruction::sdiv:
        return ConstantInt::get((int)(c_value1 / c_value2), module_);
        break;
    default:
        return nullptr;
        break;
    }
}

ConstantFP *ConstFPFolder::computefp(
    Instruction::OpID op,
    ConstantFP *value1,
    ConstantFP *value2)
{

    float c_value1 = value1->get_value();
    float c_value2 = value2->get_value();
    switch (op)
    {
    case Instruction::fadd:
        return ConstantFP::get(c_value1 + c_value2, module_);

        break;
    case Instruction::fsub:
        return ConstantFP::get(c_value1 - c_value2, module_);
        break;
    case Instruction::fmul:
        return ConstantFP::get(c_value1 * c_value2, module_);
        break;
    case Instruction::fdiv:
        return ConstantFP::get((float)(c_value1 / c_value2), module_);
        break;
    default:
        return nullptr;
        break;
    }
}

// 用来判断value是否为ConstantFP，如果不是则会返回nullptr
ConstantFP *cast_constantfp(Value *value)
{
    auto constant_fp_ptr = dynamic_cast<ConstantFP *>(value);
    if (constant_fp_ptr)
    {
        return constant_fp_ptr;
    }
    else
    {
        return nullptr;
    }
}
ConstantInt *cast_constantint(Value *value)
{
    auto constant_int_ptr = dynamic_cast<ConstantInt *>(value);
    if (constant_int_ptr)
    {
        return constant_int_ptr;
    }
    else
    {
        return nullptr;
    }
}


void ConstPropagation::run(){
    //从这里开始吧！
    //0.要考虑+ - * /还有> < ≥ ≤以及== !=
    //1.即使有未知数，我们的最终结果也是(未知数op已知常数)
    //2.要删除无用的分支，如果if的判断条件一开始就是1或着0，就跳转到对应BB块，删除无用的
    //3.while有点难办，一开始是0，直接跳转删除即可，要是1，有两个想法，1是不删让他自己跑，2是判断下
    //4.全局变量要拎出来处理的
    //5.对于while跟if找跳转判断块
    std::set<Value *> global_live_var_name;
    std::map<Value *, std::set<BasicBlock *>> live_var_2blocks;
    int tag;
    tag = 1;
    while(tag == 1){
    for(auto func_ : this->m_->get_functions()){
        if(func_->get_num_basic_blocks()!=0){
        }
        for(auto bb : func_->get_basic_blocks()){
            flag = 0;
            for(auto instr : bb->get_instructions()){
                if(instr->is_phi()){
                    flag = 1;
                    phistack[instr].push_back(instr);
                }
                if(instr->is_br()){
                    if(instr->get_num_operand() == 3 && flag == 1){//有phi，是while的判断循环块
                        judegstack[bb->get_name()].push_back(bb);
                        auto bbsucc = (BasicBlock *)(instr->get_operand(1));
                        whilestack[instr->get_operand(1)->get_name()].push_back(bbsucc);
                        //whilestack[]
                        //if的可以直接判断，我就不做特殊处理了，判断好，直接delete就行，但是while不行
                    }
                }
            }
        }
    }
    int judge;
    for(auto func_ : this->m_->get_functions()){
    std::map<std::string, std::vector<Value *>> valstack;
        for ( auto bb : func_->get_basic_blocks() ){
            judge = 0;
            std::map<std::string, std::vector<Value *>> globalstack;
            if(judegstack.find(bb->get_name())!=judegstack.end()){
                judge = 1;
            }
            for ( auto instr : bb->get_instructions() ){
                if(judge == 0){
                    int i =0;
                for(auto op : instr->get_operands()){
                    if(valstack.find(op->get_name()) != valstack.end()){
                        instr->set_operand(i,valstack[op->get_name()].back());
                    }
                    i++;
                }
                }
                if ( instr->is_cmp()){
                    //获取操作数目，找到是常数的，相加
                    if((IS_C0NST(instr->get_operand(0)) || phistack.find(instr->get_operand(0)) != phistack.end() || valstack.find(instr->get_operand(0)->get_name()) != valstack.end())&& 
                        (IS_C0NST(instr->get_operand(1))  ||phistack.find(instr->get_operand(1)) != phistack.end() || valstack.find(instr->get_operand(1)->get_name()) != valstack.end())){
                        auto a = (ConstantInt *)(instr)->get_operand(0);
                        auto b = (ConstantInt *)(instr)->get_operand(1);
                        auto type = (CmpInst*)instr;
                        auto optype = type->get_cmp_op(); 
                        auto c = (a->get_value() >= b->get_value());
                        if(optype == CmpInst::CmpOp::GE){
                            c = (a->get_value() >= b->get_value());
                        }
                        if(optype == CmpInst::CmpOp::GT){
                            c = (a->get_value() > b->get_value());
                        }
                        if(optype ==  CmpInst::CmpOp::LT){
                            c = (a->get_value() < b->get_value());
                        }
                        if(optype ==  CmpInst::CmpOp::EQ){
                            c = (a->get_value() == b->get_value());
                        }
                        if(optype ==  CmpInst::CmpOp::LE){
                            c = (a->get_value() <= b->get_value());
                        }
                        if(optype ==  CmpInst::CmpOp::NE){
                            c = (a->get_value() != b->get_value());
                        }
                        auto d = (ConstantInt*)c;
                        if(judge == 1){
                            valstack[instr->get_name()].push_back(d); 
                        }
                        if(judge == 0){    
                            valstack[instr->get_name()].push_back(CONST_INT(d));                
                            wait_delete.push_back(instr);
                        }
                    }
                }
                if(instr->is_add()){
                    if(cast_constantint(instr->get_operand(0))!=nullptr && cast_constantint(instr->get_operand(1))!= nullptr){
                        auto a = (ConstantInt *)(instr)->get_operand(0);
                        auto b = (ConstantInt *)(instr)->get_operand(1);
                        auto c = a->get_value() + b->get_value();
                        valstack[instr->get_name()].push_back(CONST_INT(c));
                        for(auto bb2 :func_->get_basic_blocks()){
                        for(auto instr2: bb2->get_instructions()){
                            if(instr2->is_phi()){
                                auto r_vals = instr2->get_operands();
                                int i = 0;
                                for(auto r_val : r_vals){
                                    auto r_str = r_val->get_name();
                                    if(valstack.find(r_str)!=valstack.end()){
                                        auto b = valstack[r_str].back();
                                        auto b_to_print = (Instruction *)b;
                                        instr2->set_operand(i, b);
                                    }
                                    i++;
                                }
                            }
                        }
                        }
                        wait_delete.push_back(instr);
                    }                    
                }
                if(instr->is_sub()){
                    if(cast_constantint(instr->get_operand(0))!=nullptr && cast_constantint(instr->get_operand(1))!= nullptr){
                        auto a = (ConstantInt *)(instr)->get_operand(0);
                        auto b = (ConstantInt *)(instr)->get_operand(1);
                        auto c = a->get_value() - b->get_value();
                        valstack[instr->get_name()].push_back(CONST_INT(c));
                        for(auto bb2 :func_->get_basic_blocks()){
                        for(auto instr2: bb2->get_instructions()){
                            if(instr2->is_phi()){
                                auto r_vals = instr2->get_operands();
                                int i = 0;
                                for(auto r_val : r_vals){
                                    auto r_str = r_val->get_name();
                                    if(valstack.find(r_str)!=valstack.end()){
                                        auto b = valstack[r_str].back();
                                        auto b_to_print = (Instruction *)b;
                                        instr2->set_operand(i, b);
                                    }
                                    i++;
                                }
                            }
                        }
                        }
                        wait_delete.push_back(instr);
                    }                    
                }          
                if(instr->is_mul()){
                    if(cast_constantint(instr->get_operand(0))!=nullptr && cast_constantint(instr->get_operand(1))!= nullptr){
                        auto a = (ConstantInt *)(instr)->get_operand(0);
                        auto b = (ConstantInt *)(instr)->get_operand(1);
                        auto c = a->get_value() * b->get_value();
                        valstack[instr->get_name()].push_back(CONST_INT(c));
                        for(auto bb2 :func_->get_basic_blocks()){
                        for(auto instr2: bb2->get_instructions()){
                            if(instr2->is_phi()){
                                auto r_vals = instr2->get_operands();
                                int i = 0;
                                for(auto r_val : r_vals){
                                    auto r_str = r_val->get_name();
                                    if(valstack.find(r_str)!=valstack.end()){
                                        auto b = valstack[r_str].back();
                                        auto b_to_print = (Instruction *)b;
                                        instr2->set_operand(i, b);
                                    }
                                    i++;
                                }
                            }
                        }
                        }
                        wait_delete.push_back(instr);
                    }
                }
                if(instr->is_div()){
                    if(cast_constantint(instr->get_operand(0))!=nullptr && cast_constantint(instr->get_operand(1))!= nullptr){
                        auto a = (ConstantInt *)(instr)->get_operand(0);
                        auto b = (ConstantInt *)(instr)->get_operand(1);
                        auto c = a->get_value() / b->get_value();
                        valstack[instr->get_name()].push_back(CONST_INT(c));
                        for(auto bb2 :func_->get_basic_blocks()){
                        for(auto instr2: bb2->get_instructions()){
                            if(instr2->is_phi()){
                                auto r_vals = instr2->get_operands();
                                int i = 0;
                                for(auto r_val : r_vals){
                                    auto r_str = r_val->get_name();
                                    if(valstack.find(r_str)!=valstack.end()){
                                        auto b = valstack[r_str].back();
                                        auto b_to_print = (Instruction *)b;
                                        instr2->set_operand(i, b);
                                    }
                                    i++;
                                }
                            }
                        }
                        }
                        wait_delete.push_back(instr);
                    }
                }
                if(instr->is_fadd()){
                    if(cast_constantfp(instr->get_operand(0))!=nullptr && cast_constantfp(instr->get_operand(1))!= nullptr){
                        auto a = (ConstantFP *)(instr)->get_operand(0);
                        auto b = (ConstantFP *)(instr)->get_operand(1);
                        auto c = a->get_value() + b->get_value();
                        valstack[instr->get_name()].push_back(CONST_FP(c));
                        for(auto bb2 :func_->get_basic_blocks()){
                        for(auto instr2: bb2->get_instructions()){
                            if(instr2->is_phi()){
                                auto r_vals = instr2->get_operands();
                                int i = 0;
                                for(auto r_val : r_vals){
                                    auto r_str = r_val->get_name();
                                    if(valstack.find(r_str)!=valstack.end()){
                                        auto b = valstack[r_str].back();
                                        auto b_to_print = (Instruction *)b;
                                        instr2->set_operand(i, b);
                                    }
                                    i++;
                                }
                            }
                        }
                        }
                        wait_delete.push_back(instr);
                    }
                }
                if(instr->is_fsub()){
                    if(cast_constantfp(instr->get_operand(0))!=nullptr && cast_constantfp(instr->get_operand(1))!= nullptr){
                        auto a = (ConstantFP *)(instr)->get_operand(0);
                        auto b = (ConstantFP *)(instr)->get_operand(1);
                        auto c = a->get_value() - b->get_value();
                        valstack[instr->get_name()].push_back(CONST_FP(c));
                        for(auto bb2 :func_->get_basic_blocks()){
                        for(auto instr2: bb2->get_instructions()){
                            if(instr2->is_phi()){
                                auto r_vals = instr2->get_operands();
                                int i = 0;
                                for(auto r_val : r_vals){
                                    auto r_str = r_val->get_name();
                                    if(valstack.find(r_str)!=valstack.end()){
                                        auto b = valstack[r_str].back();
                                        auto b_to_print = (Instruction *)b;
                                        instr2->set_operand(i, b);
                                    }
                                    i++;
                                }
                            }
                        }
                        }
                        wait_delete.push_back(instr);
                    }
                }
            //tag添加CONSTFP的判断以及冗余bb块删除
                if(instr->is_fmul()){
                    if(cast_constantfp(instr->get_operand(0))!=nullptr && cast_constantfp(instr->get_operand(1))!= nullptr){
                        auto a = (ConstantFP *)(instr)->get_operand(0);
                        auto b = (ConstantFP *)(instr)->get_operand(1);
                        auto c = a->get_value() * b->get_value();
                        valstack[instr->get_name()].push_back(CONST_FP(c));
                        for(auto bb2 :func_->get_basic_blocks()){
                        for(auto instr2: bb2->get_instructions()){
                            if(instr2->is_phi()){
                                auto r_vals = instr2->get_operands();
                                int i = 0;
                                for(auto r_val : r_vals){
                                    auto r_str = r_val->get_name();
                                    if(valstack.find(r_str)!=valstack.end()){
                                        auto b = valstack[r_str].back();
                                        auto b_to_print = (Instruction *)b;
                                        instr2->set_operand(i, b);
                                    }
                                    i++;
                                }
                            }
                        }
                        }
                        wait_delete.push_back(instr);
                    }
                }
                if(instr->is_fp2si()){
                    auto num = instr->get_num_operand();
                    auto a = (ConstantFP *)(instr)->get_operand(0);
                    int b = a->get_value();
                    valstack[instr->get_name()].push_back(CONST_INT(b));
                    for(auto bb2 :func_->get_basic_blocks()){
                        for(auto instr2: bb2->get_instructions()){
                            if(instr2->is_phi()){
                                auto r_vals = instr2->get_operands();
                                int i = 0;
                                for(auto r_val : r_vals){
                                    auto r_str = r_val->get_name();
                                    if(valstack.find(r_str)!=valstack.end()){
                                        auto b = valstack[r_str].back();
                                        auto b_to_print = (Instruction *)b;
                                        instr2->set_operand(i, b);
                                    }
                                    i++;
                                }
                            }
                        }
                    }
                    wait_delete.push_back(instr);
                }
                if(instr->is_si2fp()){
                    auto num = instr->get_num_operand();
                    auto a = (ConstantInt *)(instr)->get_operand(0);
                    int b = a->get_value();
                    valstack[instr->get_name()].push_back(CONST_FP(b));
                    for(auto bb2 :func_->get_basic_blocks()){
                        for(auto instr2: bb2->get_instructions()){
                            if(instr2->is_phi()){
                                auto r_vals = instr2->get_operands();
                                int i = 0;
                                for(auto r_val : r_vals){
                                    auto r_str = r_val->get_name();
                                    if(valstack.find(r_str)!=valstack.end()){
                                        auto b = valstack[r_str].back();
                                        auto b_to_print = (Instruction *)b;
                                        instr2->set_operand(i, b);
                                    }
                                    i++;
                                }
                            }
                        }
                    }
                    wait_delete.push_back(instr);
                }
                if(instr->is_load()){
                    auto a = instr->get_operand(0);
                    auto l_val = instr->get_name();
                    if(globalstack.find(a->get_name()) != globalstack.end()){
                        valstack[instr->get_name()].push_back(globalstack[a->get_name()].back());
                        wait_delete.push_back(instr);
                    }
                }
                if(instr->is_store()){
                    if(cast_constantint(instr->get_operand(0))!=nullptr){
                        auto r_val = (ConstantInt *)(instr)->get_operand(0);
                        auto l_val = static_cast<StoreInst *>(instr)->get_lval();
                        if(IS_GLOBAL_VARIABLE(l_val)){
                            globalstack[l_val->get_name()].push_back(CONST_INT(r_val->get_value()));
                        }
                    }else if(cast_constantfp(instr->get_operand(0))!=nullptr){
                        auto r_val = (ConstantFP *)(instr)->get_operand(0);
                        auto l_val = static_cast<StoreInst *>(instr)->get_lval();
                        if(IS_GLOBAL_VARIABLE(l_val)){
                            globalstack[l_val->get_name()].push_back(CONST_FP(r_val->get_value()));
                        }                        
                    }
                }

                if(instr->is_phi()) {
                    auto r_vals = instr->get_operands();
                    int i = 0;
                    for(auto r_val : r_vals){
                        auto r_str = r_val->get_name();
                        if(valstack.find(r_str)!=valstack.end()){
                            auto b = valstack[r_str].back();
                            instr->set_operand(i, b);
                            i++;
                        }
                    }
                }
                if(instr->is_br()){
                    if(instr->get_num_operand() == 3){
                    if(judge == 1){
                        auto a = instr->get_operand(0)->get_name();
                        if(valstack.find(a) != valstack.end()){
                            auto b = valstack[a].back();
                            if(b == 0){//进不去while的循环块，我就给删了
                                auto deletebbname = instr->get_operand(1)->get_name();
                                auto deletebb = whilestack[deletebbname].back();
                                deletestack[deletebbname].push_back(deletebb);
                                auto enterbb = (BasicBlock*)instr->get_operand(2); 
                                instr->remove_operands(0,2);
                                instr->add_operand(enterbb);
                                //func_->remove(deletebb);
                            }
                        }
                    }else{
                            if(instr->get_operand(0) == 0){
                                auto deletebbname = instr->get_operand(1)->get_name();
                                auto deletebb = (BasicBlock* )instr->get_operand(1);
                                deletestack[deletebbname].push_back(deletebb);
                                auto enterbb = (BasicBlock*)instr->get_operand(2);
                                instr->remove_operands(0,2);
                                instr->add_operand(enterbb);
                                //func_->remove(deletebb);  
                            }else{
                                auto deletebbname = instr->get_operand(2)->get_name();
                                auto deletebb = (BasicBlock* )instr->get_operand(2);
                                deletestack[bb->get_name()].push_back(bb);
                                auto enterbb = (BasicBlock*)instr->get_operand(1);
                                instr->remove_operands(0,2);
                                instr->add_operand(enterbb);
                            }
                    }
                    }
                }
            }
            tag = 0;
            for ( auto instr : bb->get_instructions() ){
                if(instr->is_phi() && judge == 0){
                    if(deletestack.find(instr->get_operand(1)->get_name()) != deletestack.end()){
                        numstack[instr].push_back(instr->get_operand(2));
                        tag = 1;
                        wait_delete.push_back(instr);
                    }else if(deletestack.find(instr->get_operand(3)->get_name()) != deletestack.end()){
                        numstack[instr].push_back(instr->get_operand(0));
                        tag = 1;
                        wait_delete.push_back(instr);
                    }
                }
                if(judge == 0){
                    int i =0;
                for(auto op : instr->get_operands()){
                    if(numstack.find(op) != numstack.end()){
                        instr->set_operand(i,numstack[op].back());
                    }
                    i++;
                }
             }
            }
            for(auto instr : wait_delete){
                bb->delete_instr(instr);
            }
        }
    }
    }
}