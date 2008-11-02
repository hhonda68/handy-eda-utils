#
# Lightweight single-clock digital logic simulator library.
#

# Copyright (c) 2008 the handy-eda-utils develeper(s).
# Distributed under the MIT License.
# (See accompanying file COPYING or copy at
#  http://www.opensource.org/licenses/mit-license.php.)

# Last modified on March 2004.

def debug(*args)
#  printf(*args)
end

class PrimitiveCircuit
  def initialize(nin, nout)
    @input = Array.new(nin)
    nin.times do |i|
      @input[i] = Array.new(3)
    end
    @output = Array.new(nout)
  end
  def connect(ix, src, srcox, n=1)
    n.times do |i|
      pin = @input[ix+i]
      pin[0] = src
      pin[1] = srcox+i
      pin[2] = nil
    end
  end
  def invalue(ix, n=1)
    debug("#{self.class} in #{ix}:#{n} check\n")
    value = 0
    n.times do |bit|
      pin = @input[ix+bit]
      val = (pin[2] ||= pin[0].outvalue(pin[1]))
      value |= (val << bit)
    end
    debug("#{self.class} in #{ix}:#{n} = #{value}\n")
    value
  end
  def outvalue(ox, n=1)
    debug("#{self.class} out #{ox}:#{n} check\n")
    value = 0
    n.times do |bit|
      oxbit = ox + bit
      val = (@output[oxbit] ||= calc_output_value(oxbit))
      value |= (val << bit)
    end
    debug("#{self.class} out #{ox}:#{n} = #{value}\n")
    value
  end
  def evaluate
  end
  def tick
    @output.size.times do |ox|
      @output[ox] = nil
    end
    @input.size.times do |ix|
      @input[ix][2] = nil
    end
  end
end

class CompoundCircuit < PrimitiveCircuit
  def initialize(nin, nout, internalcomponents)
    super(nin, nout)
    @fwin = Array.new(nin)
    @fwout = Array.new(nout)
    @internalcomponents = internalcomponents
  end
  def forward_input(ix, dst, dstix, n=1)
    n.times do
      (@fwin[ix] ||= []).push([dst, dstix])
      ix += 1
      dstix += 1
    end
  end
  def forward_output(ox, src, srcox, n=1)
    n.times do
      @fwout[ox] = [src, srcox]
      ox += 1
      srcox += 1
    end
  end
  def connect(ix, src, srcox, n=1)
    super
    n.times do |i|
      fwin = @fwin[ix+i]
      if (fwin) then
	fwin.each do |x|
	  x[0].connect(x[1], src, srcox+i)
	end
      end
    end
  end
  def outvalue(ox, n=1)
    value = 0
    n.times do |bit|
      val = @output[ox]
      unless (val) then
	fwout = @fwout[ox]
	val = fwout ? fwout[0].outvalue(fwout[1]) : calc_output_value(ox)
	@output[ox] = val
      end
      value |= (val << bit)
      ox += 1
    end
    value
  end
  def evaluate
    @internalcomponents.each do |x| x.evaluate end
  end
  def tick
    super
    @internalcomponents.each do |x| x.tick end
  end
end

################################################################

class Rng16
  def initialize(seed)
    @seed = seed
  end
  def get
    @seed = (@seed * 1103515245 + 12345) & 0xffffffff
    (@seed >> 15) & 0xffff
  end
end

class RandomBitGenerator < PrimitiveCircuit
  def initialize(width)
    super(0, width)
    @rng = Array.new(width)
    @val = Array.new(width)
    seedgen = Rng16.new(0x087221813)
    width.times do |i|
      x = seedgen.get
      x = (x << 16) | seedgen.get
      rng = Rng16.new(x)
      @rng[i] = rng
      @val[i] = rng.get
    end
    @bit = 0
  end
  def calc_output_value(ox)
    (@val[ox] >> @bit) & 1
  end
  def evaluate
  end
  def tick
    super
    if (@bit < 15) then
      @bit += 1
    else
      @val.size.times do |i|
	@val[i] = @rng[i].get
      end
      @bit = 0
    end
  end
end

class TestSignal < PrimitiveCircuit
  def initialize(width)
    super(0, width)
    @val = 0
  end
  def calc_output_value(ox)
    (@val >> ox) & 1
  end
  def set(val)
    @val = val
  end
  def get
    @val
  end
end

class Probe < PrimitiveCircuit
  Instances = []
  def initialize(width)
    super(width, 0)
    Instances.push(self)
  end
  def value
    invalue(0, @input.size)
  end
  def self.status
    Instances.map { |x| x.value }
  end
end


