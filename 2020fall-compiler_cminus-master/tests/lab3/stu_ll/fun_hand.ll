;ModuleID = 'fun.c'
source_filename = "fun.c"
;target start
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
;target end

;define callee
define dso_local i32 @callee(i32 %0) #0{
 ;mul op
 %2 = mul i32 %0, 2
 ;return the result
 ret i32 %2
}
;define end

;define main
define dso_local i32 @main() #0{
 ;call function
 %1 = call i32 @callee(i32 110)
 ;return
 ret i32 %1
}
;define end

;the following just copy the gcd_array.ll
attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.1 "}