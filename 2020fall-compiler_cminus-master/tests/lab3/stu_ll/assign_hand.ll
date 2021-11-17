;ModuleID = 'assign.c'
source_filename = "assign.c"
;target start
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
;target end

;define main
define dso_local i32 @main() #0{
 ;%1 - a[10]
 %1 = alloca [10 x i32], align 4
 ;%2 - a[0]
 %2 = getelementptr [10 x i32],[10 x i32]* %1, i32 0, i32 0
 ;store a[0] with 10
 store i32 10, i32* %2, align 4
 ;%3 - a[1]
 %3 = getelementptr [10 x i32],[10 x i32]* %1, i32 0, i32 1
 ;load a[0] to %4
 %4 = load i32, i32* %2, align 4
 ;mul
 %5 = mul i32 %4, 2
 ;store a[1] with result of mul
 store i32 %5, i32* %3, align 4
 ;load a[1] to %6
 %6 = load i32, i32* %3, align 4
 ;return %6
 ret i32 %6
}
;define end

;the following just copy the gcd_array.ll
attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.1 "}