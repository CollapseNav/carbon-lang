// Part of the Carbon Language project, under the Apache License v2.0 with LLVM
// Exceptions. See /LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// AUTOUPDATE

abstract class Abstract {
  var a: i32;
}

class Derived {
  extend base: Abstract;

  var d: i32;
}

fn Make() -> Derived {
  // TODO: This should be valid, and should construct an instance of `partial Abstract` as the base.
  // CHECK:STDERR: fail_abstract.carbon:[[@LINE+3]]:19: ERROR: Cannot construct instance of abstract class. Consider using `partial Abstract` instead.
  // CHECK:STDERR:   return {.base = {.a = 1}, .d = 7};
  // CHECK:STDERR:                   ^~~~~~~~
  return {.base = {.a = 1}, .d = 7};
}

fn Access(d: Derived) -> (i32, i32) {
  return (d.d, d.base.a);
}

// CHECK:STDOUT: --- fail_abstract.carbon
// CHECK:STDOUT:
// CHECK:STDOUT: constants {
// CHECK:STDOUT:   %.1: type = struct_type {.a: i32} [template]
// CHECK:STDOUT:   %.2: type = ptr_type {.a: i32} [template]
// CHECK:STDOUT:   %.3: type = struct_type {.base: Abstract, .d: i32} [template]
// CHECK:STDOUT:   %.4: type = struct_type {.base: {.a: i32}*, .d: i32} [template]
// CHECK:STDOUT:   %.5: type = ptr_type {.base: {.a: i32}*, .d: i32} [template]
// CHECK:STDOUT:   %.6: type = ptr_type {.base: Abstract, .d: i32} [template]
// CHECK:STDOUT:   %.7: i32 = int_literal 1 [template]
// CHECK:STDOUT:   %.8: i32 = int_literal 7 [template]
// CHECK:STDOUT:   %.9: type = struct_type {.base: {.a: i32}, .d: i32} [template]
// CHECK:STDOUT:   %.10: type = tuple_type (type, type) [template]
// CHECK:STDOUT:   %.11: type = tuple_type (i32, i32) [template]
// CHECK:STDOUT:   %.12: type = ptr_type (i32, i32) [template]
// CHECK:STDOUT: }
// CHECK:STDOUT:
// CHECK:STDOUT: file {
// CHECK:STDOUT:   package: <namespace> = namespace package, {.Abstract = %Abstract.decl, .Derived = %Derived.decl, .Make = %Make, .Access = %Access}
// CHECK:STDOUT:   %Abstract.decl = class_decl @Abstract, ()
// CHECK:STDOUT:   %Abstract: type = class_type @Abstract [template]
// CHECK:STDOUT:   %Derived.decl = class_decl @Derived, ()
// CHECK:STDOUT:   %Derived: type = class_type @Derived [template]
// CHECK:STDOUT:   %Make: <function> = fn_decl @Make [template]
// CHECK:STDOUT:   %Access: <function> = fn_decl @Access [template]
// CHECK:STDOUT: }
// CHECK:STDOUT:
// CHECK:STDOUT: class @Abstract {
// CHECK:STDOUT:   %.loc8_8.1: type = unbound_element_type Abstract, i32 [template]
// CHECK:STDOUT:   %.loc8_8.2: <unbound element of class Abstract> = field_decl a, element0 [template]
// CHECK:STDOUT:   %a: <unbound element of class Abstract> = bind_name a, %.loc8_8.2 [template = %.loc8_8.2]
// CHECK:STDOUT:
// CHECK:STDOUT: !members:
// CHECK:STDOUT:   .a = %a
// CHECK:STDOUT: }
// CHECK:STDOUT:
// CHECK:STDOUT: class @Derived {
// CHECK:STDOUT:   %Abstract.ref: type = name_ref Abstract, file.%Abstract [template = file.%Abstract]
// CHECK:STDOUT:   %.loc12_24.1: type = unbound_element_type Derived, Abstract [template]
// CHECK:STDOUT:   %.loc12_24.2: <unbound element of class Derived> = base_decl Abstract, element0 [template]
// CHECK:STDOUT:   %.loc14_8.1: type = unbound_element_type Derived, i32 [template]
// CHECK:STDOUT:   %.loc14_8.2: <unbound element of class Derived> = field_decl d, element1 [template]
// CHECK:STDOUT:   %d: <unbound element of class Derived> = bind_name d, %.loc14_8.2 [template = %.loc14_8.2]
// CHECK:STDOUT:
// CHECK:STDOUT: !members:
// CHECK:STDOUT:   .base = %.loc12_24.2
// CHECK:STDOUT:   .d = %d
// CHECK:STDOUT:   extend name_scope1
// CHECK:STDOUT: }
// CHECK:STDOUT:
// CHECK:STDOUT: fn @Make() -> %return: Derived {
// CHECK:STDOUT: !entry:
// CHECK:STDOUT:   %.loc22_25: i32 = int_literal 1 [template = constants.%.7]
// CHECK:STDOUT:   %.loc22_26: {.a: i32} = struct_literal (%.loc22_25)
// CHECK:STDOUT:   %.loc22_34: i32 = int_literal 7 [template = constants.%.8]
// CHECK:STDOUT:   %.loc22_35: {.base: {.a: i32}, .d: i32} = struct_literal (%.loc22_26, %.loc22_34)
// CHECK:STDOUT:   return <error>
// CHECK:STDOUT: }
// CHECK:STDOUT:
// CHECK:STDOUT: fn @Access(%d: Derived) -> %return: (i32, i32) {
// CHECK:STDOUT: !entry:
// CHECK:STDOUT:   %d.ref.loc26_11: Derived = name_ref d, %d
// CHECK:STDOUT:   %.loc26_12.1: ref i32 = class_element_access %d.ref.loc26_11, element1
// CHECK:STDOUT:   %.loc26_12.2: i32 = bind_value %.loc26_12.1
// CHECK:STDOUT:   %d.ref.loc26_16: Derived = name_ref d, %d
// CHECK:STDOUT:   %.loc26_17.1: ref Abstract = class_element_access %d.ref.loc26_16, element0
// CHECK:STDOUT:   %.loc26_17.2: Abstract = bind_value %.loc26_17.1
// CHECK:STDOUT:   %.loc26_22.1: ref i32 = class_element_access %.loc26_17.2, element0
// CHECK:STDOUT:   %.loc26_22.2: i32 = bind_value %.loc26_22.1
// CHECK:STDOUT:   %.loc26_24.1: (i32, i32) = tuple_literal (%.loc26_12.2, %.loc26_22.2)
// CHECK:STDOUT:   %.loc26_24.2: ref i32 = tuple_access %return, element0
// CHECK:STDOUT:   %.loc26_24.3: init i32 = initialize_from %.loc26_12.2 to %.loc26_24.2
// CHECK:STDOUT:   %.loc26_24.4: ref i32 = tuple_access %return, element1
// CHECK:STDOUT:   %.loc26_24.5: init i32 = initialize_from %.loc26_22.2 to %.loc26_24.4
// CHECK:STDOUT:   %.loc26_24.6: init (i32, i32) = tuple_init (%.loc26_24.3, %.loc26_24.5) to %return
// CHECK:STDOUT:   %.loc26_24.7: init (i32, i32) = converted %.loc26_24.1, %.loc26_24.6
// CHECK:STDOUT:   return %.loc26_24.7
// CHECK:STDOUT: }
// CHECK:STDOUT:
