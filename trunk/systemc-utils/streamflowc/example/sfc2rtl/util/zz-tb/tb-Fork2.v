// iverilog -y .. -o tb-Fork2.vvp tb-Fork2.v
// vvp tb-Fork2.vvp

`timescale 1ns/1ns
module tb;

localparam W = 8;
localparam CYC = 10;

reg          clk, rst;
reg 	     idata_vld;
wire 	     idata_rdy;
reg [W-1:0]  idata;
wire 	     odata0_vld, odata1_vld;
reg 	     odata0_rdy, odata1_rdy;
wire [W-1:0] odata0, odata1;
	    
Fork2#(W) dut(clk, rst, idata_vld, idata_rdy, idata, odata0_vld, odata0_rdy, odata0, odata1_vld, odata1_rdy, odata1);

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

initial begin :ivld
  reg [31:0] rngseed, rnum;
  integer    n;
  #0;
  idata_vld = 1'b0;
  wait_reset_release;
  rngseed = 32'h01234567;
  forever begin
    rnum = $random(rngseed);
    n = rnum[7:0];
    // 192 32 16  8  4  2  1  1
    //   0  1  2  3  4  5  6  X
    if (n >= 64) begin
      n = 0;
    end else if (n >= 32) begin
      n = 1;
    end else if (n >= 16) begin
      n = 2;
    end else if (n >= 8) begin
      n = 3;
    end else if (n >= 4) begin
      n = 4;
    end else if (n >= 2) begin
      n = 5;
    end else if (n >= 1) begin
      n = 6;
    end else begin
      n = rnum[15:8];
    end
    if (n > 0) begin
      idata_vld <= 1'b0;
      idata_update_ctrl = 1'b0;
      -> idata_update_event;
      repeat (n) @(posedge clk);
    end
    idata_vld <= 1'b1;
    idata_update_ctrl = 1'b1;
    -> idata_update_event;
    @(posedge clk);
    while (~idata_rdy)  @(posedge clk);
  end
end

initial begin :idat
  reg [31:0] rngseed, rnum;
  #0;
  idata = {W{1'b0}};
  rngseed = 32'h12345678;
  forever begin
    @(idata_update_event);
    if (idata_update_ctrl) begin
      rnum = $random(rngseed);
      idata <= rnum[W-1:0];
    end else begin
      idata <= {W{1'bx}};
    end
  end
end

initial begin :ordy0
  reg [31:0] rngseed, rnum;
  integer    n;
  #0;
  odata0_rdy = 1'b0;
  wait_reset_release;
  rngseed = 32'h23456789;
  forever begin
    rnum = $random(rngseed);
    n = rnum[7:0];
    // 192 32 16  8  4  2  1  1
    //   0  1  2  3  4  5  6  X
    if (n >= 64) begin
      n = 0;
    end else if (n >= 32) begin
      n = 1;
    end else if (n >= 16) begin
      n = 2;
    end else if (n >= 8) begin
      n = 3;
    end else if (n >= 4) begin
      n = 4;
    end else if (n >= 2) begin
      n = 5;
    end else if (n >= 1) begin
      n = 6;
    end else begin
      n = rnum[15:8];
    end
    if (n > 0) begin
      odata0_rdy <= 1'b0;
      repeat (n)  @(posedge clk);
    end
    odata0_rdy <= 1'b1;
    @(posedge clk);
  end
end

initial begin :ordy1
  reg [31:0] rngseed, rnum;
  integer    n;
  #0;
  odata1_rdy = 1'b0;
  wait_reset_release;
  rngseed = 32'h3456789a;
  forever begin
    rnum = $random(rngseed);
    n = rnum[7:0];
    // 192 32 16  8  4  2  1  1
    //   0  1  2  3  4  5  6  X
    if (n >= 64) begin
      n = 0;
    end else if (n >= 32) begin
      n = 1;
    end else if (n >= 16) begin
      n = 2;
    end else if (n >= 8) begin
      n = 3;
    end else if (n >= 4) begin
      n = 4;
    end else if (n >= 2) begin
      n = 5;
    end else if (n >= 1) begin
      n = 6;
    end else begin
      n = rnum[15:8];
    end
    if (n > 0) begin
      odata1_rdy <= 1'b0;
      repeat (n)  @(posedge clk);
    end
    odata1_rdy <= 1'b1;
    @(posedge clk);
  end
end

initial begin
  $dumpfile("tb-Fork2.vcd");
  $dumpvars(0, dut);
  #(1000*CYC);
  $display("Simulated 1000 cycles.");
  $finish;
end

initial begin :verify_idata
  reg [31:0] rngseed, rnum;
  wait_reset_release;
  rngseed = 32'h12345678;
  forever begin
    if (idata_vld & idata_rdy) begin
      rnum = $random(rngseed);
      if (idata !== rnum[W-1:0]) begin
	$display("%0t: Error: idata %h != %h", $time, idata, rnum[W-1:0]);
	#(CYC);
	$finish;
      end
      $display("%0t: %h", $time, idata);
    end
    @(posedge clk);
  end
end

initial begin :verify_odata0
  reg [31:0] rngseed, rnum;
  wait_reset_release;
  rngseed = 32'h12345678;
  forever begin
    if (odata0_vld & odata0_rdy) begin
      rnum = $random(rngseed);
      if (odata0 !== rnum[W-1:0]) begin
	$display("%0t: Error: odata0 %h != %h", $time, odata0, rnum[W-1:0]);
	#(CYC);
	$finish;
      end
      $display("%0t:      %h", $time, odata0);
    end
    @(posedge clk);
  end
end

initial begin :verify_odata1
  reg [31:0] rngseed, rnum;
  wait_reset_release;
  rngseed = 32'h12345678;
  forever begin
    if (odata1_vld & odata1_rdy) begin
      rnum = $random(rngseed);
      if (odata1 !== rnum[W-1:0]) begin
	$display("%0t: Error: odata1 %h != %h", $time, odata1, rnum[W-1:0]);
	#(CYC);
	$finish;
      end
      $display("%0t:            %h", $time, odata1);
    end
    @(posedge clk);
  end
end

endmodule
