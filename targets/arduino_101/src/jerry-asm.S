/* Copyright 2014-2015 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * setjmp
 *
 * According to x86_32 System V ABI, the following registers are
 * callee-saved, and so need to be stored in context:
 *   - %ebx
 *   - %esp
 *   - %ebp
 *   - %esi
 *   - %edi
 *   - x87 control word
 *
 * Also, we should store:
 *   - return address (to jump to upon longjmp)
 *
 * mov return_address ([%esp]) -> %eax
 *
 * mov env ([%esp + 0x4]) -> %edx
 *
 * mov %ebx -> jmp_buf_0  ([%edx + 0x0])
 * mov %esp -> jmp_buf_4  ([%edx + 0x4])
 * mov %ebp -> jmp_buf_8  ([%edx + 0x8])
 * mov %esi -> jmp_buf_12 ([%edx + 0xc])
 * mov %edi -> jmp_buf_16 ([%edx + 0x10])
 * mov %eax -> jmp_buf_20 ([%edx + 0x14])
 * fnstcw   -> jmp_buf_24 ([%edx + 0x18])
 *
 * ret
 */
#define _SETJMP \
  mov (%esp), %eax;       \
  mov 0x4 (%esp), %edx;   \
                          \
  mov %ebx, 0x00 (%edx);  \
  mov %esp, 0x04 (%edx);  \
  mov %ebp, 0x08 (%edx);  \
  mov %esi, 0x0c (%edx);  \
  mov %edi, 0x10 (%edx);  \
  mov %eax, 0x14 (%edx);  \
                          \
  xor %eax, %eax;         \
                          \
  ret

/*
 * longjmp
 *
 * See also:
 *          _SETJMP
 *
 * mov env ([%esp + 0x4]) -> %edx
 * mov val ([%esp + 0x8]) -> %eax
 *
 * mov jmp_buf_0    ([%edx + 0x0])  -> %ebx
 * mov jmp_buf_4    ([%edx + 0x8])  -> %esp
 * mov jmp_buf_8    ([%edx + 0x10]) -> %ebp
 * mov jmp_buf_12   ([%edx + 0x18]) -> %esi
 * mov jmp_buf_16   ([%edx + 0x20]) -> %edi
 * mov jmp_buf_20   ([%edx + 0x28]) -> %ecx
 * fldcw jmp_buf_24 ([%edx + 0x30])
 *
 * mov return_address (%ecx) -> ([%esp])
 *
 * cmp (%eax), 0x0
 * jnz 1f
 * xor %eax, %eax
 * 1:
 *
 * ret
 */
#define _LONGJMP \
  mov 0x4 (%esp), %edx;   \
  mov 0x8 (%esp), %eax;   \
                          \
  mov 0x0 (%edx), %ebx;   \
  mov 0x4 (%edx), %esp;   \
  mov 0x8 (%edx), %ebp;   \
  mov 0xc (%edx), %esi;   \
  mov 0x10 (%edx), %edi;  \
  mov 0x14 (%edx), %ecx;  \
                          \
  mov %ecx, (%esp);       \
                          \
  test %eax, %eax;        \
  jnz 1f;                 \
  xor %eax, %eax;         \
 1:                       \
                          \
  ret

/**
 * setjmp (jmp_buf env)
 *
 * See also:
 *          longjmp
 *
 * @return 0 - if returns from direct call,
 *         nonzero - if returns after longjmp.
 */
.global setjmp
.type setjmp, %function
setjmp:
  _SETJMP
.size setjmp, . - setjmp

/**
 * longjmp (jmp_buf env, int val)
 *
 * Note:
 *      if val is not 0, then it would be returned from setjmp,
 *      otherwise - 0 would be returned.
 *
 * See also:
 *          setjmp
 */
.global longjmp
.type longjmp, %function
longjmp:
  _LONGJMP
.size longjmp, . - setjmp
