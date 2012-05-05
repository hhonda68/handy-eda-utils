#!/usr/bin/ruby -w
#
# EMIPS : Embedding MIPS Microprocessor
#
# Copyright (c) 2012 the handy-eda-utils developer(s).
# Distributed under the MIT License.
# (See accompanying file COPYING or copy at
#  http://www.opensource.org/licenses/mit-license.php.)
#
# ruby checkinsn.rb foo.dis
#  ==> if (foo.dis contains unsupported instructions) {
#         print them and exit(2);
#       } else {
#         print nothing and exit(0);
#       }

SupportedInsnList = %w(sll srl sra sllv srlv srav jr jalr mfhi mflo mult multu div divu) +
                    %w(addu subu and or xor nor slt sltu bltz bgez bltzal bgezal j jal beq bne blez bgtz) +
                    %w(addiu slti sltiu andi ori xori lui mfc0 mtc0 lb lh lw lbu lhu sb sh sw) +
                    %w(nop move negu beqz bnez li)
SupportedInsn = {}
SupportedInsnList.each do |insn|
  SupportedInsn[insn] = true
end

exitcode = 0
while ($_ = gets) do
  ($_ =~ /^[0-9a-f]{8}:\s+[0-9a-f]{8}\s+(\w+)/)  or next
  opcode = $1
  unless SupportedInsn.include?(opcode) then
    print("Error: unsupported insn: ", $_)
    exitcode = 2
  end
end
exit(exitcode)
