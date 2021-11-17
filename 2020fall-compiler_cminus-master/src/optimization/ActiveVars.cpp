#include "ActiveVars.hpp"
#include "logging.hpp"
#include "LoopSearch.hpp"
#include <iostream>
#define IS_GLOBAL_VARIABLE(l_val) dynamic_cast<GlobalVariable *>(l_val)
void ActiveVars::run()
{
    std::ofstream output_active_vars;
    output_active_vars.open("active_vars.json", std::ios::out);
    output_active_vars << "[";
    for (auto &func : this->m_->get_functions()) {
        if (func->get_basic_blocks().empty()) {
            continue;
        }
        else
        {
            func_ = func;  
            func_->set_instr_name();
            live_in.clear();
            live_out.clear();
            std::set<Value*> args;
            std::map<BasicBlock*, std::set<Value *>> use;
            std::map<BasicBlock*, std::set<Value *>> def;
            std::map<BasicBlock*, std::set<Value *>> notuse;
            use.clear();
            def.clear();
            for(auto argument : func_->get_args()){
                args.insert(argument);
            }
            // 在此分析 func_ 的每个bb块的活跃变量，并存储在 live_in live_out 结构内
            for(auto bb : func_->get_basic_blocks()){
                for(auto instr : bb->get_instructions()){
                    if(instr->is_alloca()){
                        if(use[bb].find(instr) == use[bb].end()){
                            def[bb].insert(instr);
                        }
                    }
                    for(auto op : instr->get_operands()){
                        auto constantint = dynamic_cast<ConstantInt *>(op);
                        auto constantfp = dynamic_cast<ConstantFP *>(op);
                        if(!op->get_type()->is_label_type() &&constantint== nullptr && constantfp == nullptr && !op->get_type()->is_function_type()){
                            if(use[bb].find(op) == use[bb].end()){//在引用前没有定值
                                if(def[bb].find(op) == def[bb].end()){
                                    use[bb].insert(op);
                                }
                            }
                        }
                        if(use[bb].find(instr) == use[bb].end() && !instr->is_br() && (!instr->is_call() || (instr->is_call() && !instr->get_type()->is_void_type()))  && !instr->is_ret()){
                            //在定值前，没有引用
                            def[bb].insert(instr);
                        }
                    }
                } 
            }

            int flag = 1;
            while(flag == 1){
                flag = 0;
                for(auto bb : func_->get_basic_blocks()){
                for(auto bbsucc : bb->get_succ_basic_blocks()){//OUT[B]=IN[S]的并集 S是B的后继块
                    int a= 0;
                    for(auto instr: bbsucc->get_instructions()){
                        if(instr->is_phi()){
                                //记录phi跳转的块，以及跳转对应的值
                            if(instr->get_num_operand() == 4){
                                auto truebb = instr->get_operand(1);
                                auto trueop = instr->get_operand(0);
                                auto falsebb = instr->get_operand(3);
                                auto falseop = instr->get_operand(2);
                                if(truebb == bb && falsebb == bb){

                                }
                                if(truebb == bb && falsebb != bb){//不要falsebb的
                                    notuse[bb].insert(falseop);
                                    a = 1;
                                }
                                if(falsebb == bb && truebb != bb){
                                    notuse[bb].insert(trueop);
                                    a = 1;
                                }
                                if(truebb != bb && falsebb != bb){
                                    a = 1;
                                    notuse[bb].insert(trueop);
                                    notuse[bb].insert(falseop);
                                }
                            }else{
                                auto onlyop = instr->get_operand(0);
                                auto onlybb = instr->get_operand(1);
                                if(onlybb != bb){
                                    notuse[bb].insert(onlyop);
                                    a = 1;
                                }
                            }
                        }
                    }
                    if(a == 0){
                        for(auto insucc : live_in[bbsucc]){
                            live_out[bb].insert(insucc);
                        }
                    }else{
                        for(auto insucc : live_in[bbsucc]){
                            if(notuse[bb].find(insucc) == notuse[bb].end()){
                                live_out[bb].insert(insucc);
                            }
                        }
                        notuse[bb].clear();
                    }
                }
                for(auto notdef : live_out[bb]){
                    if(def[bb].find(notdef) == def[bb].end() && live_in[bb].find(notdef) == live_in[bb].end()){//不在def中，而且也不在live_in
                        live_in[bb].insert(notdef);
                        flag = 1;
                    }
                }
                for(auto useb : use[bb]){
                    if(live_in[bb].find(useb) == live_in[bb].end()){
                        live_in[bb].insert(useb);
                        flag = 1;
                    }
                }
            }
        }
            output_active_vars << print();
            output_active_vars << ",";
        }
    }
    output_active_vars << "]";
    output_active_vars.close();
    return ;
}

std::string ActiveVars::print()
{
    std::string active_vars;
    active_vars +=  "{\n";
    active_vars +=  "\"function\": \"";
    active_vars +=  func_->get_name();
    active_vars +=  "\",\n";

    active_vars +=  "\"live_in\": {\n";
    for (auto &p : live_in) {
        if (p.second.size() == 0) {
            continue;
        } else {
            active_vars +=  "  \"";
            active_vars +=  p.first->get_name();
            active_vars +=  "\": [" ;
            for (auto &v : p.second) {
                active_vars +=  "\"%";
                active_vars +=  v->get_name();
                active_vars +=  "\",";
            }
            active_vars += "]" ;
            active_vars += ",\n";   
        }
    }
    active_vars += "\n";
    active_vars +=  "    },\n";
    
    active_vars +=  "\"live_out\": {\n";
    for (auto &p : live_out) {
        if (p.second.size() == 0) {
            continue;
        } else {
            active_vars +=  "  \"";
            active_vars +=  p.first->get_name();
            active_vars +=  "\": [" ;
            for (auto &v : p.second) {
                active_vars +=  "\"%";
                active_vars +=  v->get_name();
                active_vars +=  "\",";
            }
            active_vars += "]";
            active_vars += ",\n";
        }
    }
    active_vars += "\n";
    active_vars += "    }\n";

    active_vars += "}\n";
    active_vars += "\n";
    return active_vars;
}