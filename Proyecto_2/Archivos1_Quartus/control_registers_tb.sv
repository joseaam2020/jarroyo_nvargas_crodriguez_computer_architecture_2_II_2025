import fixed_point_pkg::*;

module tb_control_registers;

    logic clk, rst_n;
    logic [7:0] addr;
    logic wr_en, rd_en;
    logic [31:0] wr_data, rd_data;

    logic [9:0] img_width, img_height;
    fixed_point_t scale_factor;
    logic mode_simd;
    logic [2:0] simd_width;
    logic start, step_mode, step_next;
    logic busy, ready, error;
    logic [31:0] progress, flops_count, mem_reads, mem_writes;

    control_registers dut (
        .clk(clk),
        .rst_n(rst_n),
        .addr(addr),
        .wr_en(wr_en),
        .rd_en(rd_en),
        .wr_data(wr_data),
        .rd_data(rd_data),
        .img_width(img_width),
        .img_height(img_height),
        .scale_factor(scale_factor),
        .mode_simd(mode_simd),
        .simd_width(simd_width),
        .start(start),
        .step_mode(step_mode),
        .step_next(step_next),
        .busy(1'b0),
        .ready(1'b1),
        .error(1'b0),
        .progress(32'h0),
        .flops_count(32'hDEADBEEF),
        .mem_reads(32'h00001234),
        .mem_writes(32'h00005678)
    );

    initial begin
        clk = 1'b0;
        forever #5 clk = ~clk;
    end

    initial begin
        $display("=== Control Registers Testbench ===\n");

        rst_n = 1'b0;
        wr_en = 1'b0;
        rd_en = 1'b0;
        addr = 8'h0;
        #10;
        rst_n = 1'b1;
        #10;

        $display("Test 1: Write Image Width");
        addr = 8'h00;
        wr_data = 32'h0200;
        wr_en = 1'b1;
        #10;
        wr_en = 1'b0;
        #10;
        if (img_width == 10'd512) begin
            $display("  img_width = 512: PASS\n");
        end else begin
            $display("  img_width = %d: FAIL\n", img_width);
        end

        $display("Test 2: Write Image Height");
        addr = 8'h01;
        wr_data = 32'h0100;
        wr_en = 1'b1;
        #10;
        wr_en = 1'b0;
        #10;
        if (img_height == 10'd256) begin
            $display("  img_height = 256: PASS\n");
        end else begin
            $display("  img_height = %d: FAIL\n", img_height);
        end

        $display("Test 3: Write Scale Factor");
        addr = 8'h02;
        wr_data = 32'h0080;
        wr_en = 1'b1;
        #10;
        wr_en = 1'b0;
        #10;
        if (scale_factor == 16'h0080) begin
            $display("  scale_factor = 0x0080: PASS\n");
        end else begin
            $display("  scale_factor = 0x%h: FAIL\n", scale_factor);
        end

        $display("Test 4: Write Mode");
        addr = 8'h03;
        wr_data = 32'h00000005;
        wr_en = 1'b1;
        #10;
        wr_en = 1'b0;
        #10;
        if (mode_simd == 1'b1 && simd_width == 3'd2) begin
            $display("  mode_simd = 1, simd_width = 2: PASS\n");
        end else begin
            $display("  mode_simd = %b, simd_width = %d: FAIL\n", mode_simd, simd_width);
        end

        $display("Test 5: Read Performance Counters");
        addr = 8'h07;
        rd_en = 1'b1;
        #10;
        if (rd_data == 32'hDEADBEEF) begin
            $display("  flops_count = 0xDEADBEEF: PASS\n");
        end else begin
            $display("  flops_count = 0x%h: FAIL\n", rd_data);
        end

        addr = 8'h08;
        #10;
        if (rd_data == 32'h00001234) begin
            $display("  mem_reads = 0x00001234: PASS\n");
        end else begin
            $display("  mem_reads = 0x%h: FAIL\n", rd_data);
        end

        rd_en = 1'b0;
        #10;

        $display("=== All Control Register Tests Complete ===\n");
        $finish;
    end

endmodule