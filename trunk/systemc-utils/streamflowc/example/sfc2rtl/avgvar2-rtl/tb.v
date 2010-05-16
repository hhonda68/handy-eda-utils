// iverilog -y . -y ../util -o tb.vvp tb.v
// vvp tb.vvp

`timescale 1ns/1ns
module tb;

localparam CYC = 10;

reg          clk, rst;
reg 	     idata_vld;
wire 	     idata_rdy;
reg [7:0]    idata;
wire 	     odata_vld;
reg 	     odata_rdy;
wire [23:0]  odata;

AvgVar dut(clk, rst, idata_vld, idata_rdy, idata, odata_vld, odata_rdy, odata);

initial begin
  #0;
  rst = 1'b1;
  #(10*CYC+CYC/3);
  rst = 1'b0;
end

initial begin
  #0;
  clk = 1'b0;
  #(CYC);
  forever begin
    clk = 1'b1;
    #(CYC/2);
    clk = 1'b0;
    #(CYC-CYC/2);
  end
end

task wait_reset_release;
begin
  @(posedge clk);
  while (rst)  @(posedge clk);
end
endtask

event idata_update_event;
reg   idata_update_ctrl;

reg [31:0] ivld_rnum;
wire [7:0] ivld_nr_wait_cycle;
calc_nr_wait_cycle ivld_nr_wait(ivld_rnum, ivld_nr_wait_cycle);

initial begin :ivld
  reg [31:0] rngseed;
  reg pace;
  pace = $test$plusargs("pace_ivld");
  #0;
  idata_vld = 1'b0;
  wait_reset_release;
  rngseed = 32'h01234567;
  forever begin
    if (pace) begin
      ivld_rnum = $random(rngseed);
      #0;
      if (ivld_nr_wait_cycle > 0) begin
	idata_vld <= 1'b0;
	idata_update_ctrl = 1'b0;
	-> idata_update_event;
	repeat (ivld_nr_wait_cycle) @(posedge clk);
      end
    end
    idata_vld <= 1'b1;
    idata_update_ctrl = 1'b1;
    -> idata_update_event;
    @(posedge clk);
    while (~idata_rdy)  @(posedge clk);
  end
end

reg [31:0] global_rngseed = 32'h12345678;
task global_rng_get;
output [7:0] val;
begin
  global_rngseed = global_rngseed * 32'd1103515245 + 32'd12345;
  val = global_rngseed[22:15];
end
endtask // global_rng_get

initial begin :idat
  reg [7:0] rnum;
  #0;
  idata = 8'h00;
  forever begin
    @(idata_update_event);
    if (idata_update_ctrl) begin
      global_rng_get(rnum);
      idata <= rnum;
    end else begin
      idata <= 8'hxx;
    end
  end
end

reg [31:0] ordy_rnum;
wire [7:0] ordy_nr_wait_cycle;
calc_nr_wait_cycle ordy_nr_wait(ordy_rnum, ordy_nr_wait_cycle);

initial begin :ordy
  reg [31:0] rngseed;
  reg pace;
  pace = $test$plusargs("pace_ordy");
  #0;
  odata_rdy = 1'b0;
  wait_reset_release;
  if (pace) begin
    rngseed = 32'h76543210;
    forever begin
      ordy_rnum = $random(rngseed);
      #0;
      if (ordy_nr_wait_cycle > 0) begin
	odata_rdy <= 1'b0;
	repeat (ordy_nr_wait_cycle)  @(posedge clk);
      end
      odata_rdy <= 1'b1;
      @(posedge clk);
    end
  end else begin
    odata_rdy <= 1'b1;
  end
end

initial begin
  $dumpfile("tb.vcd");
  $dumpvars(0, dut);
  #(10000*CYC);
  $display("Simulated 10000 cycles.");
  $finish;
end

initial begin :print
  integer inum = 0;
  integer onum = 0;
  wait_reset_release;
  forever begin
    if (idata_vld & idata_rdy) begin
      $display("%0t: d%0d %0d", $time, inum, idata);
      inum = inum + 1;
    end
    if (odata_vld & odata_rdy) begin
      $display("%0t: avg %0d var %0d", $time, odata[23:16], odata[15:0]);
      onum = onum + 1;
      if (onum == 100) begin
	#(CYC);
	$finish;
      end
    end
    @(posedge clk);
  end
end

endmodule // tb

module calc_nr_wait_cycle(rnum, nr_wait_cycle);
input [31:0] rnum;
output reg [7:0] nr_wait_cycle;
always @(rnum) begin :body
  // probability    nr_wait_cycle
  // -----------------------------
  //   192/256            0
  //    32/256            1
  //    16/256            2
  //     8/256            3
  //     4/256            4
  //     2/256            5
  //     1/256            6
  //     1/256          0-255
  reg [7:0] n;
  n = rnum[7:0];
  if (n >= 64) begin
    nr_wait_cycle = 0;
  end else if (n >= 32) begin
    nr_wait_cycle = 1;
  end else if (n >= 16) begin
    nr_wait_cycle = 2;
  end else if (n >= 8) begin
    nr_wait_cycle = 3;
  end else if (n >= 4) begin
    nr_wait_cycle = 4;
  end else if (n >= 2) begin
    nr_wait_cycle = 5;
  end else if (n >= 1) begin
    nr_wait_cycle = 6;
  end else begin
    nr_wait_cycle = rnum[15:8];
  end
end
endmodule // calc_nr_wait_cycle
