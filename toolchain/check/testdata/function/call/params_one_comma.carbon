// Part of the Carbon Language project, under the Apache License v2.0 with LLVM
// Exceptions. See /LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// AUTOUPDATE

fn Foo(a: i32,) {}

fn Main() {
  Foo(1);
  Foo(1,);
}

// CHECK:STDOUT: --- params_one_comma.carbon
// CHECK:STDOUT:
// CHECK:STDOUT: constants {
// CHECK:STDOUT:   %.1: i32 = int_literal 1 [template]
// CHECK:STDOUT:   %.2: type = tuple_type () [template]
// CHECK:STDOUT:   %.3: i32 = int_literal 1 [template]
// CHECK:STDOUT: }
// CHECK:STDOUT:
// CHECK:STDOUT: file {
// CHECK:STDOUT:   package: <namespace> = namespace package, {.Foo = %Foo, .Main = %Main}
// CHECK:STDOUT:   %Foo: <function> = fn_decl @Foo [template]
// CHECK:STDOUT:   %Main: <function> = fn_decl @Main [template]
// CHECK:STDOUT: }
// CHECK:STDOUT:
// CHECK:STDOUT: fn @Foo(%a: i32) {
// CHECK:STDOUT: !entry:
// CHECK:STDOUT:   return
// CHECK:STDOUT: }
// CHECK:STDOUT:
// CHECK:STDOUT: fn @Main() {
// CHECK:STDOUT: !entry:
// CHECK:STDOUT:   %Foo.ref.loc10: <function> = name_ref Foo, file.%Foo [template = file.%Foo]
// CHECK:STDOUT:   %.loc10_7: i32 = int_literal 1 [template = constants.%.1]
// CHECK:STDOUT:   %.loc10_6: init () = call %Foo.ref.loc10(%.loc10_7)
// CHECK:STDOUT:   %Foo.ref.loc11: <function> = name_ref Foo, file.%Foo [template = file.%Foo]
// CHECK:STDOUT:   %.loc11_7: i32 = int_literal 1 [template = constants.%.3]
// CHECK:STDOUT:   %.loc11_6: init () = call %Foo.ref.loc11(%.loc11_7)
// CHECK:STDOUT:   return
// CHECK:STDOUT: }
// CHECK:STDOUT:
