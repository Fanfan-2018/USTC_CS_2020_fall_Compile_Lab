;ModuleID = 'while.c'
source_filename = "while.c"
;target start
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
;target end

;define main
define dso_local i32 @main() #0{
 ;%1 - a
 %1 = alloca i32, align 4
 ;%2 - i
 %2 = alloca i32, align 4
 ;store a with 10
 store i32 10, i32* %1, align 4
 ;store i with 0
 store i32 0, i32* %2, align 4
 ;load a to %3
 %3 = load i32, i32* %1, align 4
 ;load i to %4 hhh not necessary
 ;qiang po zheng
 %4 = load i32, i32* %2, align 4
 ;cmp
 %5 = icmp slt i32 %4, 10
 br i1 %5, label %6, label %12

6:                          ;label 6
 ;load i to %7
 %7 = load i32, i32* %2, align 4
 ;load a to %8 
 %8 = load i32, i32* %1, align 4
 ;i++
 %9 = add i32 %7, 1
 ;a+i
 %10 = add i32 %8, 1
 ;store a with the value of %10
 store i32 %10, i32* %1
 ;store i with the value of %9
 store i32 %9, i32* %2
 %11 = icmp slt i32 %9, 10
 br i1 %11, label %6, label %12 

12:                         ;label 12
 ;load a to %13
 %13 = load i32, i32* %1, align 4
 ;return a
 ret i32 %13
}
;define end

;the following just copy the gcd_array.ll
attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.1 "}