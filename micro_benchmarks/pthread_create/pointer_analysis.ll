; ModuleID = 'pointer_analysis.cpp'
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

declare void @_ZNSt8ios_base4InitC1Ev(%"class.std::ios_base::Init"*) #0

declare void @_ZNSt8ios_base4InitD1Ev(%"class.std::ios_base::Init"*) #0

; Function Attrs: nounwind
declare i32 @__cxa_atexit(void (i8*)*, i8*, i8*) #1

; Function Attrs: uwtable
define void @_Z14displayMessagei(i32 %tid) #2 {
entry:
  %call1.i = tail call dereferenceable(272) %"class.std::basic_ostream"* @_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l(%"class.std::basic_ostream"* nonnull dereferenceable(272) @_ZSt4cout, i8* nonnull getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i64 0, i64 0), i64 7)
  %call1 = tail call dereferenceable(272) %"class.std::basic_ostream"* @_ZNSolsEi(%"class.std::basic_ostream"* nonnull @_ZSt4cout, i32 %tid)
  %call1.i.5 = tail call dereferenceable(272) %"class.std::basic_ostream"* @_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l(%"class.std::basic_ostream"* nonnull dereferenceable(272) %call1, i8* nonnull getelementptr inbounds ([14 x i8], [14 x i8]* @.str.1, i64 0, i64 0), i64 13)
  %0 = bitcast %"class.std::basic_ostream"* %call1 to i8**
  %vtable.i = load i8*, i8** %0, align 8, !tbaa !1
  %vbase.offset.ptr.i = getelementptr i8, i8* %vtable.i, i64 -24
  %1 = bitcast i8* %vbase.offset.ptr.i to i64*
  %vbase.offset.i = load i64, i64* %1, align 8
  %2 = bitcast %"class.std::basic_ostream"* %call1 to i8*
  %add.ptr.i = getelementptr inbounds i8, i8* %2, i64 %vbase.offset.i
  %_M_ctype.i = getelementptr inbounds i8, i8* %add.ptr.i, i64 240
  %3 = bitcast i8* %_M_ctype.i to %"class.std::ctype"**
  %4 = load %"class.std::ctype"*, %"class.std::ctype"** %3, align 8, !tbaa !4
  %tobool.i.11 = icmp eq %"class.std::ctype"* %4, null
  br i1 %tobool.i.11, label %if.then.i.12, label %_ZSt13__check_facetISt5ctypeIcEERKT_PS3_.exit

if.then.i.12:                                     ; preds = %entry
  tail call void @_ZSt16__throw_bad_castv() #6
  unreachable

_ZSt13__check_facetISt5ctypeIcEERKT_PS3_.exit:    ; preds = %entry
  %_M_widen_ok.i = getelementptr inbounds %"class.std::ctype", %"class.std::ctype"* %4, i64 0, i32 8
  %5 = load i8, i8* %_M_widen_ok.i, align 8, !tbaa !9
  %tobool.i = icmp eq i8 %5, 0
  br i1 %tobool.i, label %if.end.i, label %if.then.i

if.then.i:                                        ; preds = %_ZSt13__check_facetISt5ctypeIcEERKT_PS3_.exit
  %arrayidx.i = getelementptr inbounds %"class.std::ctype", %"class.std::ctype"* %4, i64 0, i32 9, i64 10
  %6 = load i8, i8* %arrayidx.i, align 1, !tbaa !11
  br label %_ZNKSt5ctypeIcE5widenEc.exit

if.end.i:                                         ; preds = %_ZSt13__check_facetISt5ctypeIcEERKT_PS3_.exit
  tail call void @_ZNKSt5ctypeIcE13_M_widen_initEv(%"class.std::ctype"* nonnull %4)
  %7 = bitcast %"class.std::ctype"* %4 to i8 (%"class.std::ctype"*, i8)***
  %vtable.i.9 = load i8 (%"class.std::ctype"*, i8)**, i8 (%"class.std::ctype"*, i8)*** %7, align 8, !tbaa !1
  %vfn.i = getelementptr inbounds i8 (%"class.std::ctype"*, i8)*, i8 (%"class.std::ctype"*, i8)** %vtable.i.9, i64 6
  %8 = load i8 (%"class.std::ctype"*, i8)*, i8 (%"class.std::ctype"*, i8)** %vfn.i, align 8
  %call.i.10 = tail call signext i8 %8(%"class.std::ctype"* nonnull %4, i8 signext 10)
  br label %_ZNKSt5ctypeIcE5widenEc.exit

_ZNKSt5ctypeIcE5widenEc.exit:                     ; preds = %if.then.i, %if.end.i
  %retval.0.i = phi i8 [ %6, %if.then.i ], [ %call.i.10, %if.end.i ]
  %call1.i.7 = tail call dereferenceable(272) %"class.std::basic_ostream"* @_ZNSo3putEc(%"class.std::basic_ostream"* nonnull %call1, i8 signext %retval.0.i)
  %call.i = tail call dereferenceable(272) %"class.std::basic_ostream"* @_ZNSo5flushEv(%"class.std::basic_ostream"* nonnull %call1.i.7)
  ret void
}

declare dereferenceable(272) %"class.std::basic_ostream"* @_ZNSolsEi(%"class.std::basic_ostream"*, i32) #0

; Function Attrs: uwtable
define noalias i8* @_Z14workerFunctionPv(i8* nocapture readonly %args) #2 {
entry:
  %tid = bitcast i8* %args to i32*
  %0 = load i32, i32* %tid, align 4, !tbaa !12
  tail call void @_Z14displayMessagei(i32 %0)
  ret i8* null
}

; Function Attrs: nounwind argmemonly
declare void @llvm.lifetime.start(i64, i8* nocapture) #3

; Function Attrs: nounwind argmemonly
declare void @llvm.lifetime.end(i64, i8* nocapture) #3

; Function Attrs: uwtable
define i32 @main() #2 {
entry:
  %wargs = alloca [2 x %struct.worker_args_t], align 4
  %0 = bitcast [2 x %struct.worker_args_t]* %wargs to i8*
  call void @llvm.lifetime.start(i64 8, i8* %0) #1
  %tid = getelementptr inbounds [2 x %struct.worker_args_t], [2 x %struct.worker_args_t]* %wargs, i64 0, i64 0, i32 0
  store i32 0, i32* %tid, align 4, !tbaa !12
  %tid2 = getelementptr inbounds [2 x %struct.worker_args_t], [2 x %struct.worker_args_t]* %wargs, i64 0, i64 1, i32 0
  store i32 1, i32* %tid2, align 4, !tbaa !12
  %1 = bitcast [2 x %struct.worker_args_t]* %wargs to i8*
  %call = call i32 @pthread_create(i64* getelementptr inbounds ([2 x i64], [2 x i64]* @t, i64 0, i64 0), %union.pthread_attr_t* null, i8* (i8*)* nonnull @_Z14workerFunctionPv, i8* %1) #1
  %arrayidx5.1 = getelementptr inbounds [2 x %struct.worker_args_t], [2 x %struct.worker_args_t]* %wargs, i64 0, i64 1
  %2 = bitcast %struct.worker_args_t* %arrayidx5.1 to i8*
  %call.1 = call i32 @pthread_create(i64* getelementptr inbounds ([2 x i64], [2 x i64]* @t, i64 0, i64 1), %union.pthread_attr_t* null, i8* (i8*)* nonnull @_Z14workerFunctionPv, i8* %2) #1
  %3 = load i64, i64* getelementptr inbounds ([2 x i64], [2 x i64]* @t, i64 0, i64 0), align 16, !tbaa !15
  %call13 = call i32 @pthread_join(i64 %3, i8** null)
  %4 = load i64, i64* getelementptr inbounds ([2 x i64], [2 x i64]* @t, i64 0, i64 1), align 8, !tbaa !15
  %call13.1 = call i32 @pthread_join(i64 %4, i8** null)
  call void @llvm.lifetime.end(i64 8, i8* %0) #1
  ret i32 0
}

; Function Attrs: nounwind
declare i32 @pthread_create(i64*, %union.pthread_attr_t*, i8* (i8*)*, i8*) #4

declare i32 @pthread_join(i64, i8**) #0

declare dereferenceable(272) %"class.std::basic_ostream"* @_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l(%"class.std::basic_ostream"* dereferenceable(272), i8*, i64) #0

declare dereferenceable(272) %"class.std::basic_ostream"* @_ZNSo3putEc(%"class.std::basic_ostream"*, i8 signext) #0

declare dereferenceable(272) %"class.std::basic_ostream"* @_ZNSo5flushEv(%"class.std::basic_ostream"*) #0

; Function Attrs: noreturn
declare void @_ZSt16__throw_bad_castv() #5

declare void @_ZNKSt5ctypeIcE13_M_widen_initEv(%"class.std::ctype"*) #0

define internal void @_GLOBAL__sub_I_pointer_analysis.cpp() #0 section ".text.startup" {
entry:
  tail call void @_ZNSt8ios_base4InitC1Ev(%"class.std::ios_base::Init"* nonnull @_ZStL8__ioinit)
  %0 = tail call i32 @__cxa_atexit(void (i8*)* bitcast (void (%"class.std::ios_base::Init"*)* @_ZNSt8ios_base4InitD1Ev to void (i8*)*), i8* getelementptr inbounds (%"class.std::ios_base::Init", %"class.std::ios_base::Init"* @_ZStL8__ioinit, i64 0, i32 0), i8* nonnull @__dso_handle) #1
  ret void
}

attributes #0 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind argmemonly }
attributes #4 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { noreturn "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #6 = { noreturn }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 248940) (llvm/trunk 248934)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"vtable pointer", !3, i64 0}
!3 = !{!"Simple C/C++ TBAA"}
!4 = !{!5, !6, i64 240}
!5 = !{!"_ZTSSt9basic_iosIcSt11char_traitsIcEE", !6, i64 216, !7, i64 224, !8, i64 225, !6, i64 232, !6, i64 240, !6, i64 248, !6, i64 256}
!6 = !{!"any pointer", !7, i64 0}
!7 = !{!"omnipotent char", !3, i64 0}
!8 = !{!"bool", !7, i64 0}
!9 = !{!10, !7, i64 56}
!10 = !{!"_ZTSSt5ctypeIcE", !6, i64 16, !8, i64 24, !6, i64 32, !6, i64 40, !6, i64 48, !7, i64 56, !7, i64 57, !7, i64 313, !7, i64 569}
!11 = !{!7, !7, i64 0}
!12 = !{!13, !14, i64 0}
!13 = !{!"_ZTS13worker_args_t", !14, i64 0}
!14 = !{!"int", !7, i64 0}
!15 = !{!16, !16, i64 0}
!16 = !{!"long", !7, i64 0}
