`timescale 1ns/1ps

module top_downscaler_simd_stepping_tb;

  // ------------------------------------------------------------------
  // Configuraciones básicas
  // ------------------------------------------------------------------
  localparam integer CLK_PERIOD_NS = 10;
  localparam integer MAX_CYCLES = 1200;

  localparam logic [7:0]
    ADDR_IMG_WIDTH  = 8'h00,
    ADDR_IMG_HEIGHT = 8'h01,
    ADDR_SCALE      = 8'h02,
    ADDR_MODE       = 8'h03,
    ADDR_CONTROL    = 8'h04;

  localparam logic [31:0]
    CTRL_START      = 32'h1,
    CTRL_STEP_MODE  = 32'h2,
    CTRL_STEP_NEXT  = 32'h1 << 8;

  // ------------------------------------------------------------------
  // Señales de testbench
  // ------------------------------------------------------------------
  logic        clk;
  logic        rst_n;
  logic [7:0]  reg_addr;
  logic        reg_wr_en;
  logic        reg_rd_en;
  logic [31:0] reg_wr_data;
  logic [31:0] reg_rd_data;
  logic        ready;
  logic        busy;
  logic        error;

  integer      cycle_cnt;
  integer      step_pulses;
  logic        seen_busy;
  logic        was_waiting;

  // ------------------------------------------------------------------
  // DUT (SIMD)
  // ------------------------------------------------------------------
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
    .mem_wr_addr(),
    .mem_wr_en(),
    .mem_wr_data(),
    .ready(ready),
    .busy(busy),
    .error(error)
  );

  // ------------------------------------------------------------------
  // Señales internas para monitor
  // ------------------------------------------------------------------
  wire [2:0] fsm_state        = dut.fsm.state;
  wire       addr_gen_valid   = dut.addr_gen_valid;
  wire       interp_valid_out = dut.interp_valid_out;
  wire       result_write_en  = dut.result_write_en;
  wire [7:0] lanes_active     = dut.lane_valid;
  wire       in_step_wait     = (fsm_state == 3'd4);

  // ------------------------------------------------------------------
  // Reloj
  // ------------------------------------------------------------------
  initial begin
    clk = 1'b0;
    forever #(CLK_PERIOD_NS / 2) clk = ~clk;
  end

  // ------------------------------------------------------------------
  // Tareas auxiliares
  // ------------------------------------------------------------------
  task write_reg(input logic [7:0] addr, input logic [31:0] data);
    begin
      @(posedge clk);
      reg_addr    <= addr;
      reg_wr_data <= data;
      reg_wr_en   <= 1;
      reg_rd_en   <= 0;

      @(posedge clk);
      reg_wr_en   <= 0;
      reg_wr_data <= 0;
    end
  endtask

  task pulse_step_next();
    begin
      write_reg(ADDR_CONTROL, CTRL_STEP_MODE | CTRL_STEP_NEXT);
    end
  endtask

  task configure_and_start();
    begin
      write_reg(ADDR_IMG_WIDTH,  32'd16);
      write_reg(ADDR_IMG_HEIGHT, 32'd16);
      write_reg(ADDR_SCALE,      32'h0100);
      write_reg(ADDR_MODE,       32'd8);

      $display("\n▶ Activando STEP MODE...\n");
      write_reg(ADDR_CONTROL, CTRL_START | CTRL_STEP_MODE);
    end
  endtask

  // ------------------------------------------------------------------
  // Helpers
  // ------------------------------------------------------------------
  function bit processing_done();
    processing_done = (ready && !busy);
  endfunction

  // ------------------------------------------------------------------
  // Debug limpio — solo imprime cambios importantes
  // ------------------------------------------------------------------

  // FSM
  logic [2:0] prev_state;
  always @(posedge clk) begin
    if (fsm_state !== prev_state) begin
      $display("[%0t] FSM cambio %0d → %0d",
               $time, prev_state, fsm_state);
      prev_state <= fsm_state;
    end
  end

  // Busy / Ready
  logic prev_busy, prev_ready;
  always @(posedge clk) begin
    if (busy !== prev_busy || ready !== prev_ready) begin
      $display("[%0t] busy=%b ready=%b", $time, busy, ready);
      prev_busy  <= busy;
      prev_ready <= ready;
    end
  end

  // Lanes activos
  reg [7:0] prev_lanes;
  always @(posedge clk) begin
    if (lanes_active !== prev_lanes) begin
      $display("[%0t] lanes activos = 0b%b (%0d)",
               $time, lanes_active, lanes_active);
      prev_lanes <= lanes_active;
    end
  end

  // Resumen periódico
  always @(posedge clk) begin
    if (cycle_cnt % 20 == 0) begin
      $display("RESUMEN ciclo %0d: state=%0d busy=%b ready=%b lanes=%b stepWait=%b",
               cycle_cnt, fsm_state, busy, ready, lanes_active, in_step_wait);
    end
  end

  // ------------------------------------------------------------------
  // Banco de pruebas principal
  // ------------------------------------------------------------------
  initial begin
    rst_n      = 1'b0;
    reg_wr_en  = 1'b0;
    reg_rd_en  = 1'b0;
    reg_addr   = 8'h0;
    reg_wr_data = 32'h0;
    reg_rd_data = 32'h0;

    repeat (6) @(posedge clk);
    rst_n = 1'b1;
    repeat (6) @(posedge clk);

    configure_and_start();

    cycle_cnt = 0;
    step_pulses = 0;
    seen_busy = 0;
    was_waiting = 0;

    step_loop: while (cycle_cnt < MAX_CYCLES) begin
      @(posedge clk);
      cycle_cnt++;

      if (busy)
        seen_busy = 1;

      if (in_step_wait && !was_waiting) begin
        was_waiting = 1;
        pulse_step_next();
        step_pulses++;
      end else if (!in_step_wait) begin
        was_waiting = 0;
      end

      if (processing_done() && seen_busy) begin
        $display("\n✓ Procesamiento terminado tras %0d ciclos y %0d pulsos.\n",
                  cycle_cnt, step_pulses);
        disable step_loop;
      end
    end

    if (cycle_cnt >= MAX_CYCLES)
      $display("\n✗ Tiempo máximo alcanzado (%0d ciclos).", MAX_CYCLES);

    $finish;
  end

endmodule
