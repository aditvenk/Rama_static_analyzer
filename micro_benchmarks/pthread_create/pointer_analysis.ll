; ModuleID = '<stdin>'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class.std::ios_base::Init" = type { i8 }
%"class.std::basic_ostream" = type { i32 (...)**, %"class.std::basic_ios" }
%"class.std::basic_ios" = type { %"class.std::ios_base", %"class.std::basic_ostream"*, i8, i8, %"class.std::basic_streambuf"*, %"class.std::ctype"*, %"class.std::num_put"*, %"class.std::num_get"* }
%"class.std::ios_base" = type { i32 (...)**, i64, i64, i32, i32, i32, %"struct.std::ios_base::_Callback_list"*, %"struct.std::ios_base::_Words", [8 x %"struct.std::ios_base::_Words"], i32, %"struct.std::ios_base::_Words"*, %"class.std::locale" }
%"struct.std::ios_base::_Callback_list" = type { %"struct.std::ios_base::_Callback_list"*, void (i32, %"class.std::ios_base"*, i32)*, i32, i32 }
%"struct.std::ios_base::_Words" = type { i8*, i64 }
%"class.std::locale" = type { %"class.std::locale::_Impl"* }
%"class.std::locale::_Impl" = type { i32, %"class.std::locale::facet"**, i64, %"class.std::locale::facet"**, i8** }
%"class.std::locale::facet" = type <{ i32 (...)**, i32, [4 x i8] }>
%"class.std::basic_streambuf" = type { i32 (...)**, i8*, i8*, i8*, i8*, i8*, i8*, %"class.std::locale" }
%"class.std::ctype" = type <{ %"class.std::locale::facet.base", [4 x i8], %struct.__locale_struct*, i8, [7 x i8], i32*, i32*, i16*, i8, [256 x i8], [256 x i8], i8, [6 x i8] }>
%"class.std::locale::facet.base" = type <{ i32 (...)**, i32 }>
%struct.__locale_struct = type { [13 x %struct.__locale_data*], i16*, i32*, i32*, [13 x i8*] }
%struct.__locale_data = type opaque
%"class.std::num_put" = type { %"class.std::locale::facet.base", [4 x i8] }
%"class.std::num_get" = type { %"class.std::locale::facet.base", [4 x i8] }
%struct.worker_args_t = type { i32 }
%union.pthread_attr_t = type { i64, [48 x i8] }

@_ZStL8__ioinit = internal global %"class.std::ios_base::Init" zeroinitializer, align 1
@__dso_handle = external global i8
@t = global [2 x i64] zeroinitializer, align 16
@_ZSt4cout = external global %"class.std::basic_ostream", align 8
@.str = private unnamed_addr constant [8 x i8] c"Worker \00", align 1
@.str.1 = private unnamed_addr constant [14 x i8] c" saying hello\00", align 1
@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 65535, void ()* @_GLOBAL__sub_I_pointer_analysis.cpp, i8* null }]

define internal void @__cxx_global_var_init() #0 section ".text.startup" {
entry:
  call void @_ZNSt8ios_base4InitC1Ev(%"class.std::ios_base::Init"* @_ZStL8__ioinit)
  %0 = call i32 @__cxa_atexit(void (i8*)* bitcast (void (%"class.std::ios_base::Init"*)* @_ZNSt8ios_base4InitD1Ev to void (i8*)*), i8* getelementptr inbounds (%"class.std::ios_base::Init", %"class.std::ios_base::Init"* @_ZStL8__ioinit, i32 0, i32 0), i8* @__dso_handle) #1
  ret void
}

declare void @_ZNSt8ios_base4InitC1Ev(%"class.std::ios_base::Init"*) #0

declare void @_ZNSt8ios_base4InitD1Ev(%"class.std::ios_base::Init"*) #0

; Function Attrs: nounwind
declare i32 @__cxa_atexit(void (i8*)*, i8*, i8*) #1

; Function Attrs: uwtable
define void @_Z14displayMessagei(i32 %tid) #2 {
entry:
  %call = call dereferenceable(272) %"class.std::basic_ostream"* @_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc(%"class.std::basic_ostream"* dereferenceable(272) @_ZSt4cout, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i32 0, i32 0))
  %call1 = call dereferenceable(272) %"class.std::basic_ostream"* @_ZNSolsEi(%"class.std::basic_ostream"* %call, i32 %tid)
  %call2 = call dereferenceable(272) %"class.std::basic_ostream"* @_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc(%"class.std::basic_ostream"* dereferenceable(272) %call1, i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str.1, i32 0, i32 0))
  %call3 = call dereferenceable(272) %"class.std::basic_ostream"* @_ZNSolsEPFRSoS_E(%"class.std::basic_ostream"* %call2, %"class.std::basic_ostream"* (%"class.std::basic_ostream"*)* @_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_)
  ret void
}

declare dereferenceable(272) %"class.std::basic_ostream"* @_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc(%"class.std::basic_ostream"* dereferenceable(272), i8*) #0

declare dereferenceable(272) %"class.std::basic_ostream"* @_ZNSolsEi(%"class.std::basic_ostream"*, i32) #0

declare dereferenceable(272) %"class.std::basic_ostream"* @_ZNSolsEPFRSoS_E(%"class.std::basic_ostream"*, %"class.std::basic_ostream"* (%"class.std::basic_ostream"*)*) #0

declare dereferenceable(272) %"class.std::basic_ostream"* @_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_(%"class.std::basic_ostream"* dereferenceable(272)) #0

; Function Attrs: uwtable
define i8* @_Z14workerFunctionPv(i8* %args) #2 {
entry:
  %0 = bitcast i8* %args to %struct.worker_args_t*
  %tid = getelementptr inbounds %struct.worker_args_t, %struct.worker_args_t* %0, i32 0, i32 0
  %1 = load i32, i32* %tid, align 4
  call void @_Z14displayMessagei(i32 %1)
  ret i8* null
}

; Function Attrs: uwtable
define i32 @main() #2 {
entry:
  %wargs = alloca [2 x %struct.worker_args_t], align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %cmp = icmp slt i32 %i.0, 2
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %idxprom = sext i32 %i.0 to i64
  %arrayidx = getelementptr inbounds [2 x %struct.worker_args_t], [2 x %struct.worker_args_t]* %wargs, i64 0, i64 %idxprom
  %tid = getelementptr inbounds %struct.worker_args_t, %struct.worker_args_t* %arrayidx, i32 0, i32 0
  store i32 %i.0, i32* %tid, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  br label %for.cond.2

for.cond.2:                                       ; preds = %for.inc.9, %for.end
  %i1.0 = phi i32 [ 0, %for.end ], [ %inc10, %for.inc.9 ]
  %cmp3 = icmp slt i32 %i1.0, 2
  br i1 %cmp3, label %for.body.4, label %for.end.11

for.body.4:                                       ; preds = %for.cond.2
  %idxprom5 = sext i32 %i1.0 to i64
  %arrayidx6 = getelementptr inbounds [2 x i64], [2 x i64]* @t, i64 0, i64 %idxprom5
  %idxprom7 = sext i32 %i1.0 to i64
  %arrayidx8 = getelementptr inbounds [2 x %struct.worker_args_t], [2 x %struct.worker_args_t]* %wargs, i64 0, i64 %idxprom7
  %0 = bitcast %struct.worker_args_t* %arrayidx8 to i8*
  %call = call i32 @pthread_create(i64* %arrayidx6, %union.pthread_attr_t* null, i8* (i8*)* @_Z14workerFunctionPv, i8* %0) #1
  br label %for.inc.9

for.inc.9:                                        ; preds = %for.body.4
  %inc10 = add nsw i32 %i1.0, 1
  br label %for.cond.2

for.end.11:                                       ; preds = %for.cond.2
  br label %for.cond.13

for.cond.13:                                      ; preds = %for.inc.19, %for.end.11
  %i12.0 = phi i32 [ 0, %for.end.11 ], [ %inc20, %for.inc.19 ]
  %cmp14 = icmp slt i32 %i12.0, 2
  br i1 %cmp14, label %for.body.15, label %for.end.21

for.body.15:                                      ; preds = %for.cond.13
  %idxprom16 = sext i32 %i12.0 to i64
  %arrayidx17 = getelementptr inbounds [2 x i64], [2 x i64]* @t, i64 0, i64 %idxprom16
  %1 = load i64, i64* %arrayidx17, align 8
  %call18 = call i32 @pthread_join(i64 %1, i8** null)
  br label %for.inc.19

for.inc.19:                                       ; preds = %for.body.15
  %inc20 = add nsw i32 %i12.0, 1
  br label %for.cond.13

for.end.21:                                       ; preds = %for.cond.13
  ret i32 0
}

; Function Attrs: nounwind
declare i32 @pthread_create(i64*, %union.pthread_attr_t*, i8* (i8*)*, i8*) #3

declare i32 @pthread_join(i64, i8**) #0

define internal void @_GLOBAL__sub_I_pointer_analysis.cpp() #0 section ".text.startup" {
entry:
  call void @__cxx_global_var_init()
  ret void
}

attributes #0 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 248940) (llvm/trunk 248934)"}
