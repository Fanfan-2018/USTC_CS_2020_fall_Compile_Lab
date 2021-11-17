#include <algorithm>
#include "logging.hpp"
#include "LoopSearch.hpp"
#include "LoopInvHoist.hpp"

int i;
void LoopInvHoist::run()
{
    i=0;
    LoopSearch loop_searcher(m_, false);
    loop_searcher.run();
    std::set<Value *> left;
    for(auto x=loop_searcher.begin();x!=loop_searcher.end();x++)  {
        int need = 1;
        for(auto bb1: **x) {
            for(auto instr1:bb1->get_instructions()){
                auto l_val = (Value *)instr1;
                left.insert(l_val);
            }
        }
        for(auto bb2: **x){
            auto instrs = bb2->get_instructions();
            
            for(auto instr2:instrs){
                auto r_vals = instr2->get_operands();
                int tol=0;
                for(auto r_val : r_vals){
                    auto r_to_print=(Instruction *)r_val;
                    auto constant_ptr = dynamic_cast<Constant *>(r_val);
                    if(constant_ptr)    continue;
                    if(left.find(r_val)!=left.end())
                    {
                        tol++;
                    }
                }
                if(instr2->is_phi()==0&&instr2->is_br()==0&&instr2->is_ret()==0&&instr2->is_cmp()==0&&instr2->is_zext()==0)
                {
                    if(tol==0){
                            auto q=*x;
                            auto base = loop_searcher.get_loop_base(q);
                            auto pre = base->get_pre_basic_blocks();
                            for(auto bb3: pre){
                                if(q->find(bb3)==q->end()) { 
                                    auto ins = bb3->get_instructions();
                                    auto des_in = instr2;
                                    for(auto in: ins){
                                        if(in->is_br()==1)  {
                                            des_in = in;
                                            bb3->delete_instr(in);
                                            break;
                                        } 
                                    }
                                    bb3->add_instruction(instr2);
                                    bb3->add_instruction(des_in);
                                    bb2->delete_instr(instr2);
                                    i=1;
                                }
                            }
                }
                }
            }
        }
    }
    if(i!=0) run();
    // 接下来由你来补充啦！
}
