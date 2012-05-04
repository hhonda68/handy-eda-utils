/*
 * EMIPS : Embedding MIPS Microprocessor
 *
 * Copyright (c) 2012 the handy-eda-utils developer(s).
 * Distributed under the MIT License.
 * (See accompanying file COPYING or copy at
 *  http://www.opensource.org/licenses/mit-license.php.)
 */

   .comm InitStack, 512

   .text
   .align 2
   .global _start
   .ent _start
_start:
   .set noreorder
   la    $gp, _gp             #initialize global pointer
   la    $sp, InitStack+488   #initialize stack pointer
   jal   main
   nop
exit:
   mtc0  $0,$31	
$L1:
   j $L1
   nop
   .end _start

   .global myputc
   .ent myputc
myputc:
   .set noreorder
   mtc0  $a0,$0
   jr    $ra
   nop
   .end myputc

