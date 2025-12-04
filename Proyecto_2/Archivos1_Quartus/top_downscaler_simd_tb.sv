`timescale 1ns/1ps

module tb_top_downscaler_simd;

  // ========================================================
  // Señales
  // ========================================================
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

  // ========================================================
  // DUT
  // ========================================================
  top_downscaler_simd #(
      .SIMD_WIDTH(8)
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

  // ========================================================
  // Reloj
  // ========================================================
  always #5 clk = ~clk;

  // ========================================================
  // TAREA: escribir registro
  // ========================================================
  task write_reg;
    input [7:0] addr;
    input [31:0] data;
    begin
      @(posedge clk);
      reg_addr    <= addr;
      reg_wr_data <= data;
      reg_wr_en   <= 1;
      reg_rd_en   <= 0;

      @(posedge clk);
      reg_wr_en   <= 0;
    end
  endtask

  // ========================================================
  // TAREA: esperar a que ready sea 1
  // ========================================================
  task wait_ready;
    begin
      while (ready == 0) @(posedge clk);
    end
  endtask

  // ========================================================
  // DUMP PARA GTKWave (si lo usas)
  // ========================================================
  initial begin
    $dumpfile("waves.vcd");
    $dumpvars(0, tb_top_downscaler_simd);
  end

  // ========================================================
  // TEST
  // ========================================================
  initial begin
    clk = 0;
    rst_n = 0;

    reg_addr = 0;
    reg_wr_en = 0;
    reg_rd_en = 0;
    reg_wr_data = 0;

    // reset
    repeat (5) @(posedge clk);
    rst_n = 1;

    // configurar registros
    write_reg(8'h00, 64);              // width
    write_reg(8'h04, 64);              // height
    write_reg(8'h08, 32'h00018000);    // scale factor = 1.5
    write_reg(8'h0C, 1);               // SIMD enable
    write_reg(8'h10, 8);               // SIMD_WIDTH
    write_reg(8'h14, 1);               // START = 1

    // esperar hasta ready se active
    wait_ready();

    $display(" ---- FIN DEL TEST ---- ");
    $stop;
  end

endmodule