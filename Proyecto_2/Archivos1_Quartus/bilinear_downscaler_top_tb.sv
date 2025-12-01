`timescale 1ns/1ps
import fixed_point_pkg::*;

module bilinear_downscaler_top_tb;
  logic clk, rst_n;
  logic [7:0] ctrl_addr;
  logic ctrl_wr_en, ctrl_rd_en;
  logic [31:0] ctrl_wr_data, ctrl_rd_data;
  logic processing_active, processing_complete;
  
  logic [31:0] read_val;
  integer cycle_count;
  integer pixels_processed;
  
  // Señales de monitoreo interno
  logic internal_busy, internal_ready, internal_error;
  logic addr_gen_start, addr_gen_done, addr_gen_valid;
  logic addr_gen_next;
  logic [3:0] fetch_state;
  logic interpolate_valid_in, interpolate_valid_out;
  logic result_write_en;
  logic [9:0] dst_x, dst_y;
  logic [9:0] dst_width, dst_height;
  
  bilinear_downscaler_top #(
    .ADDR_WIDTH(18), 
    .DATA_WIDTH(8), 
    .MEM_SIZE(262144), 
    .MAX_SIMD_WIDTH(8), 
    .CTRL_ADDR_WIDTH(8)
  ) dut (
    .clk(clk), 
    .rst_n(rst_n), 
    .ctrl_addr(ctrl_addr), 
    .ctrl_wr_en(ctrl_wr_en), 
    .ctrl_rd_en(ctrl_rd_en), 
    .ctrl_wr_data(ctrl_wr_data), 
    .ctrl_rd_data(ctrl_rd_data),
    .processing_active(processing_active), 
    .processing_complete(processing_complete)
  );
  
  // Monitoreo de señales internas
  assign internal_busy = dut.busy;
  assign internal_ready = dut.ready;
  assign internal_error = dut.error;
  assign addr_gen_start = dut.addr_gen_start;
  assign addr_gen_done = dut.addr_gen_done;
  assign addr_gen_valid = dut.addr_gen_valid;
  assign addr_gen_next = dut.addr_gen_next;
  assign fetch_state = dut.fetch_state;
  assign interpolate_valid_in = dut.interpolate_valid_in;
  assign interpolate_valid_out = dut.interpolate_valid_out;
  assign result_write_en = dut.result_write_en;
  assign dst_x = dut.addr_gen_inst.dst_x;
  assign dst_y = dut.addr_gen_inst.dst_y;
  assign dst_width = dut.addr_gen_inst.dst_width;
  assign dst_height = dut.addr_gen_inst.dst_height;
  
  // Clock generation
  initial begin
    clk = 1'b0;
    forever #5 clk = ~clk;
  end
  
  // Reset task
  task reset_system();
    $display("\n═══════════════════════════════════════════════════════════");
    $display("                      RESET SYSTEM");
    $display("═══════════════════════════════════════════════════════════");
    rst_n = 1'b0; 
    ctrl_wr_en = 1'b0; 
    ctrl_rd_en = 1'b0;
    ctrl_addr = 8'h0; 
    ctrl_wr_data = 32'h0;
    repeat(10) @(posedge clk);
    rst_n = 1'b1;
    repeat(10) @(posedge clk);
    $display("✓ Reset complete. State: busy=%b, ready=%b\n", internal_busy, internal_ready);
  endtask
  
  // Write register
  task write_register(input logic [7:0] addr, input logic [31:0] data);
    @(posedge clk);
    ctrl_addr = addr; 
    ctrl_wr_data = data; 
    ctrl_wr_en = 1'b1;
    @(posedge clk);
    ctrl_wr_en = 1'b0;
    repeat(2) @(posedge clk);
  endtask
  
  // Read register
  task read_register(input logic [7:0] addr, output logic [31:0] data);
    @(posedge clk);
    ctrl_addr = addr; 
    ctrl_rd_en = 1'b1;
    @(posedge clk);
    @(posedge clk);
    data = ctrl_rd_data; 
    ctrl_rd_en = 1'b0;
  endtask
  
  // Configure and run test
  task run_test(
    input string test_name,
    input integer width,
    input integer height,
    input integer scale,      // 0x0080 = 0.5x, 0x0100 = 1.0x, etc.
    input integer simd_w,
    input integer timeout
  );
    integer expected_pixels;
    real scale_float;
    
    $display("\n╔══════════════════════════════════════════════════════════╗");
    $display("║  %s", test_name);
    $display("╚══════════════════════════════════════════════════════════╝");
    
    // Calculate expected output pixels
    scale_float = scale / 256.0;
    expected_pixels = int'((width * scale_float) * (height * scale_float));
    
    $display("Configuration:");
    $display("  • Input size:  %0dx%0d", width, height);
    $display("  • Scale:       0x%04h (%.2fx)", scale, scale_float);
    $display("  • SIMD width:  %0d", simd_w);
    $display("  • Expected output: %0dx%0d = %0d pixels", 
             int'(width * scale_float), int'(height * scale_float), expected_pixels);
    
    // Configure
    write_register(8'h00, width);
    write_register(8'h01, height);
    write_register(8'h02, scale);
    write_register(8'h03, simd_w);
    
    repeat(5) @(posedge clk);
    
    // Start
    $display("\n▶ Starting processing...");
    write_register(8'h04, 32'h00000001);
    
    repeat(10) @(posedge clk);
    
    // Wait for busy
    cycle_count = 0;
    while (!internal_busy && cycle_count < 100) begin
      @(posedge clk);
      cycle_count++;
    end
    
    if (!internal_busy) begin
      $display("✗ ERROR: Processing never started!");
      return;
    end
    
    $display("✓ Processing started at cycle %0d", cycle_count);
    $display("\nMonitoring pipeline (reporting every cycle):\n");
    $display("Cycle | Fetch | AddrNext | InterpIn | InterpOut | WriteEn | Position | Done");
    $display("------|-------|----------|----------|-----------|---------|----------|-----");
    
    // Monitor processing cycle by cycle
    pixels_processed = 0;
    cycle_count = 0;
    
    while (processing_active && cycle_count < timeout) begin
      @(posedge clk);
      cycle_count++;
      
      // Count pixels written
      if (result_write_en && interpolate_valid_out)
        pixels_processed++;
      
      // Report EVERY cycle for small images
      if (cycle_count <= 200 || (cycle_count % 10 == 0)) begin
        $display("%5d | %5d |    %b     |    %b     |     %b     |    %b    | (%3d,%3d) |  %b",
                 cycle_count, fetch_state, addr_gen_next, interpolate_valid_in,
                 interpolate_valid_out, result_write_en, dst_x, dst_y, addr_gen_done);
      end
    end
    
    $display("------|-------|----------|----------|-----------|---------|----------|-----");
    
    if (cycle_count >= timeout) begin
      $display("\n✗ FAILED: Timeout after %0d cycles", timeout);
      $display("  Pixels processed: %0d / %0d", pixels_processed, expected_pixels);
      $display("  Final position: (%0d, %0d)", dst_x, dst_y);
      $display("  Final fetch state: %0d", fetch_state);
    end else begin
      $display("\n✓ SUCCESS: Processing completed in %0d cycles", cycle_count);
      $display("  Pixels processed: %0d / %0d", pixels_processed, expected_pixels);
      
      if (pixels_processed == expected_pixels)
        $display("  ✓ Pixel count matches expected!");
      else
        $display("  ⚠ Pixel count mismatch!");
    end
    
    // Read performance counters
    $display("\nPerformance Metrics:");
    read_register(8'h07, read_val);
    $display("  • FLOPS:        %0d", read_val);
    read_register(8'h08, read_val);
    $display("  • Memory reads: %0d", read_val);
    read_register(8'h09, read_val);
    $display("  • Memory writes: %0d", read_val);
    
    repeat(20) @(posedge clk);
  endtask
  
  // Main test sequence
  initial begin
    $display("\n");
    $display("████████████████████████████████████████████████████████████");
    $display("█                                                          █");
    $display("█   Bilinear Downscaler - MINIMAL IMAGE TESTBENCH         █");
    $display("█                                                          █");
    $display("████████████████████████████████████████████████████████████");
    
    pixels_processed = 0;
    
    // ========================================================================
    // TEST 1: Ultra-tiny 4x4 -> 2x2 (4 output pixels)
    // ========================================================================
    reset_system();
    run_test(
      .test_name("TEST 1: Ultra-Tiny 4x4 → 2x2 (Sequential)"),
      .width(4),
      .height(4),
      .scale(32'h0080),    // 0.5x
      .simd_w(32'h0000),   // Sequential
      .timeout(500)
    );
    
    // ========================================================================
    // TEST 2: Tiny 4x4 -> 4x4 (16 output pixels, 1:1)
    // ========================================================================
    reset_system();
    run_test(
      .test_name("TEST 2: Tiny 4x4 → 4x4 (Sequential, 1:1)"),
      .width(4),
      .height(4),
      .scale(32'h0100),    // 1.0x
      .simd_w(32'h0000),   // Sequential
      .timeout(1000)
    );
    
    // ========================================================================
    // TEST 3: Small 8x8 -> 4x4 (16 output pixels)
    // ========================================================================
    reset_system();
    run_test(
      .test_name("TEST 3: Small 8x8 → 4x4 (Sequential)"),
      .width(8),
      .height(8),
      .scale(32'h0080),    // 0.5x
      .simd_w(32'h0000),   // Sequential
      .timeout(1000)
    );
    
    // ========================================================================
    // TEST 4: Small 8x8 -> 4x4 with SIMD width=2
    // ========================================================================
    reset_system();
    run_test(
      .test_name("TEST 4: Small 8x8 → 4x4 (SIMD width=2)"),
      .width(8),
      .height(8),
      .scale(32'h0080),    // 0.5x
      .simd_w(32'h0002),   // SIMD width = 2
      .timeout(1000)
    );
    
    // ========================================================================
    // TEST 5: 16x16 -> 8x8 (64 output pixels)
    // ========================================================================
    reset_system();
    run_test(
      .test_name("TEST 5: Medium 16x16 → 8x8 (SIMD width=4)"),
      .width(16),
      .height(16),
      .scale(32'h0080),    // 0.5x
      .simd_w(32'h0004),   // SIMD width = 4
      .timeout(3000)
    );
    
    $display("\n");
    $display("████████████████████████████████████████████████████████████");
    $display("█                                                          █");
    $display("█                  ALL TESTS COMPLETED                     █");
    $display("█                                                          █");
    $display("████████████████████████████████████████████████████████████");
    $display("\n");
    
    repeat(100) @(posedge clk);
    $finish;
  end
  
  // Monitor critical events
  always @(posedge clk) begin
    if (processing_complete && !internal_busy)
      $display("\n    ⏹  Processing Complete Signal");
  end
  
  // Track state transitions
  logic prev_busy;
  initial prev_busy = 1'b0;
  
  always @(posedge clk) begin
    if (internal_busy && !prev_busy)
      $display("\n    ▶  FSM entered BUSY state");
    if (!internal_busy && prev_busy)
      $display("\n    ⏸  FSM left BUSY state");
    prev_busy <= internal_busy;
  end

endmodule