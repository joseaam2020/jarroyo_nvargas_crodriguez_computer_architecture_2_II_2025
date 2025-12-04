`timescale 1ns/1ps

module tb_mem_test;

  // ---------------------------------------------------------
  // Señales del DUT
  // ---------------------------------------------------------
  logic clk;
  logic rst_n;

  logic [7:0]  reg_addr;
  logic        reg_wr_en;
  logic        reg_rd_en;
  logic [31:0] reg_wr_data;
  logic [31:0] reg_rd_data;

  logic [17:0] mem_wr_addr;
  logic        mem_wr_en;
  logic [63:0] mem_wr_data;

  logic ready, busy, error;

  // ---------------------------------------------------------
  // Instanciar DUT
  // ---------------------------------------------------------
  top_downscaler_simd #(
    .SIMD_WIDTH(8),
    .ADDR_WIDTH(18),
    .COORD_WIDTH(10),
    .REG_ADDR_WIDTH(8),
    .MEM_SIZE(16384)
  ) dut (
    .clk(clk),
    .rst_n(rst_n),
    .reg_addr(reg_addr),
    .reg_wr_en(reg_wr_en),
    .reg_rd_en(reg_rd_en),
    .reg_wr_data(reg_wr_data),
    .reg_rd_data(reg_rd_data),
    .mem_wr_addr(mem_wr_addr),
    .mem_wr_en(mem_wr_en),
    .mem_wr_data(mem_wr_data),
    .ready(ready),
    .busy(busy),
    .error(error)
  );

  // ---------------------------------------------------------
  // Clock
  // ---------------------------------------------------------
  initial clk = 0;
  always #5 clk = ~clk;

  // ---------------------------------------------------------
  // Tareas auxiliares
  // ---------------------------------------------------------
  task apb_write(input [7:0] addr, input [31:0] data);
    @(posedge clk);
    reg_addr   = addr;
    reg_wr_en  = 1;
    reg_rd_en  = 0;
    reg_wr_data = data;
    @(posedge clk);
    reg_wr_en  = 0;
  endtask

  task apb_read(input [7:0] addr);
    @(posedge clk);
    reg_addr   = addr;
    reg_rd_en  = 1;
    reg_wr_en  = 0;
    @(posedge clk);
    $display("Read reg[%0d] = %h", addr, reg_rd_data);
    reg_rd_en  = 0;
  endtask

  // ---------------------------------------------------------
  // Fuerza de memoria (A/B testing)
  // ---------------------------------------------------------
  task force_memory_pattern(input bit sel);
    if (sel == 0) begin
      // TEST A = AA
      force dut.mem_rd_data_tl = {8{8'hAA}};
      force dut.mem_rd_data_tr = {8{8'hAA}};
      force dut.mem_rd_data_bl = {8{8'hAA}};
      force dut.mem_rd_data_br = {8{8'hAA}};
      $display("=== TEST A: memoria = AA AA AA AA ===");
    end else begin
      // TEST B = 55
      force dut.mem_rd_data_tl = {8{8'h55}};
      force dut.mem_rd_data_tr = {8{8'h55}};
      force dut.mem_rd_data_bl = {8{8'h55}};
      force dut.mem_rd_data_br = {8{8'h55}};
      $display("=== TEST B: memoria = 55 55 55 55 ===");
    end
  endtask

  // ---------------------------------------------------------
  // Reset
  // ---------------------------------------------------------
  initial begin
    rst_n = 0;
    reg_wr_en = 0;
    reg_rd_en = 0;
    reg_addr  = 0;
    reg_wr_data = 0;

    #20;
    rst_n = 1;
  end

  // ---------------------------------------------------------
  // ESTÍMULOS PRINCIPALES
  // ---------------------------------------------------------
  initial begin
    @(posedge rst_n);

    // Configuración mínima
    apb_write(0, 32'd128);          // img_width
    apb_write(1, 32'd128);          // img_height
    apb_write(2, 32'h0001_0000);    // scale_factor = 1.0 Q16.16
    apb_write(3, 32'd1);            // modo SIMD
    apb_write(4, 32'd8);            // SIMD width
    apb_write(5, 32'd0);            // step_mode

    // -----------------------------------------------------
    // TEST A
    // -----------------------------------------------------
    force_memory_pattern(0);
    apb_write(6, 32'd1);  // start

    wait(ready);
    $display("FIN TEST A | ready=%0d busy=%0d error=%0d", ready, busy, error);

    #20;

    // -----------------------------------------------------
    // TEST B
    // -----------------------------------------------------
    force_memory_pattern(1);
    apb_write(6, 32'd1);  // start

    wait(ready);
    $display("FIN TEST B | ready=%0d busy=%0d error=%0d", ready, busy, error);

    #50;

    $display("=== FIN DE SIM ===");
    $stop;
  end

endmodule
