;ModuleID = 'if.c'
source_filename = "if.c"
;target start
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
;target end

;define main
define dso_local i32 @main() #0{
 ;%1 - a
 %1 = alloca float
 ;store float* %1 with 5.555
 store float 0x40163851E0000000, float* %1
 ;load %1 to %2
 %2 = load float, float* %1 
 ;if(a > 1)
 %3 = fcmp ogt float %2 ,1.000000e+00
 ;a > 1 than return 233; else return 0
 br i1 %3, label %4, label %7
 
4:                      ;label4
 ;%5 - a
 %5 = alloca i32
 ;store %5 with 233
 store i32 233, i32* %5
 ;load %5 to %6
 %6 = load i32, i32* %5
 ;return %6
 ret i32 %6

7:                      ;label7
 ;%8 - a
 %8 = alloca i32 
 ;store %8 with 0
 store i32 0, i32* %8
 ;load %8 to %9
 %9 = load i32, i32* %8
 ;return %9
 ret i32 %9
}
;define end

;the following just copy the gcd_array.ll
attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.1 "}