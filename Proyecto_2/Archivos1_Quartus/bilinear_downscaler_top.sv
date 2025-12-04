`timescale 1ns/1ps

import fixed_point_pkg::*;

module bilinear_downscaler_top #(
  parameter ADDR_WIDTH = 18,
  parameter DATA_WIDTH = 8,
  parameter MEM_SIZE = 262144,
  parameter MAX_SIMD_WIDTH = 8,
  parameter CTRL_ADDR_WIDTH = 8
)(
  input  logic clk,
  input  logic rst_n,
  input  logic [CTRL_ADDR_WIDTH-1:0] ctrl_addr,
  input  logic ctrl_wr_en,
  input  logic ctrl_rd_en,
  input  logic [31:0] ctrl_wr_data,
  output logic [31:0] ctrl_rd_data,
  output logic processing_active,
  output logic processing_complete
);

  // ---------------------------------------------------------------------------
  // Señales de control
  // ---------------------------------------------------------------------------
  logic [9:0] img_width, img_height;
  fixed_point_t scale_factor;
  logic mode_simd;
  logic [2:0] simd_width;               // número de lanes activas (1..MAX_SIMD_WIDTH), 0->interpretado como 1
  logic start, step_mode, step_next;
  logic busy, ready, error;
  logic [31:0] progress, flops_count, mem_reads, mem_writes;

  // ---------------------------------------------------------------------------
  // Señales internas
  // ---------------------------------------------------------------------------
  logic addr_gen_start, addr_gen_next, addr_gen_valid, addr_gen_done;
  logic [ADDR_WIDTH-1:0] addr_tl, addr_tr, addr_bl, addr_br;
  fixed_point_t weight_x, weight_y;

  // simd_pixels: 4 vecinos (TL, TR, BL, BR), cada uno contiene MAX_SIMD_WIDTH pixels * DATA_WIDTH bits
  logic [MAX_SIMD_WIDTH*DATA_WIDTH-1:0] simd_pixels [0:3];

  // interp_valid: un bit por lane (0..MAX_SIMD_WIDTH-1). interp_valid[i] viene de interpolador i
  logic [MAX_SIMD_WIDTH-1:0] interp_valid;
  logic [MAX_SIMD_WIDTH-1:0] interp_valid_masked;
  logic interpolate_valid_in, interpolate_valid_out;

  // result write
  logic result_write_en;

  logic [ADDR_WIDTH-1:0] result_write_addr;
  logic [ADDR_WIDTH-1:0] src_mem_rd_addr;
  logic [DATA_WIDTH-1:0] src_mem_rd_data;
  logic [ADDR_WIDTH-1:0] dst_mem_wr_addr;
  logic dst_mem_wr_en;
  logic [DATA_WIDTH-1:0] dst_mem_wr_data;

  // Contadores y FSM fetch
  logic [9:0] pixel_counter_x, pixel_counter_y;
  logic [2:0] fetch_state;

  // salida SIMD
  logic simd_valid;
  logic [MAX_SIMD_WIDTH*DATA_WIDTH-1:0] simd_pixel_out;

  localparam FETCH_IDLE = 3'd0, FETCH_TL = 3'd1, FETCH_TR = 3'd2,
             FETCH_BL   = 3'd3, FETCH_BR = 3'd4, FETCH_DONE = 3'd5;

  // ---------------------------------------------------------------------------
  // Control Registers (puertos nombrados para evitar confusiones)
  // ---------------------------------------------------------------------------
  control_registers #(.ADDR_WIDTH(CTRL_ADDR_WIDTH)) ctrl_regs_inst (
    .clk(clk), .rst_n(rst_n), .addr(ctrl_addr), .wr_en(ctrl_wr_en), .rd_en(ctrl_rd_en),
    .wr_data(ctrl_wr_data), .rd_data(ctrl_rd_data),
    .img_width(img_width), .img_height(img_height), .scale_factor(scale_factor),
    .mode_simd(mode_simd), .simd_width(simd_width),
    .start(start), .step_mode(step_mode), .step_next(step_next),
    .busy(busy), .ready(ready), .error(error),
    .progress(progress), .flops_count(flops_count), .mem_reads(mem_reads), .mem_writes(mem_writes)
  );

  // ---------------------------------------------------------------------------
  // FSM
  // ---------------------------------------------------------------------------
  downscaler_fsm fsm_inst (
    .clk(clk), .rst_n(rst_n),
    .start(start), .step_mode(step_mode), .step_next(step_next),
    .addr_gen_done(addr_gen_done),
    .addr_gen_start(addr_gen_start), .addr_gen_next(addr_gen_next),
    .interpolate_start(),                 // conectar si tu FSM lo requiere
    .interpolate_done(interpolate_valid_out),
    .result_write_en(result_write_en),
    .busy(busy), .ready(ready), .error(error)
  );

  // ---------------------------------------------------------------------------
  // Address Generator
  // ---------------------------------------------------------------------------
  address_generator addr_gen_inst (
    .clk(clk), .rst_n(rst_n),
    .start(addr_gen_start), .next_pixel(addr_gen_next),
    .src_width(img_width), .src_height(img_height), .scale_factor(scale_factor),
    .simd_width(simd_width),
    .addr_tl(addr_tl), .addr_tr(addr_tr), .addr_bl(addr_bl), .addr_br(addr_br),
    .weight_x(weight_x), .weight_y(weight_y),
    .valid(addr_gen_valid), .done(addr_gen_done)
  );

  // ---------------------------------------------------------------------------
  // Memories (source / destination)
  // ---------------------------------------------------------------------------
  image_memory #(.ADDR_WIDTH(ADDR_WIDTH), .DATA_WIDTH(DATA_WIDTH), .MEM_SIZE(MEM_SIZE)) src_memory (
    .clk(clk), .rd_addr(src_mem_rd_addr), .rd_data(src_mem_rd_data),
    .wr_addr({ADDR_WIDTH{1'b0}}), .wr_en(1'b0), .wr_data({DATA_WIDTH{1'b0}})
  );

  image_memory #(.ADDR_WIDTH(ADDR_WIDTH), .DATA_WIDTH(DATA_WIDTH), .MEM_SIZE(MEM_SIZE)) dst_memory (
    .clk(clk), .rd_addr({ADDR_WIDTH{1'b0}}), .rd_data(),
    .wr_addr(dst_mem_wr_addr), .wr_en(dst_mem_wr_en), .wr_data(dst_mem_wr_data)
  );

  // ---------------------------------------------------------------------------
  // SIMD Output Registers (opcional: según tu implementación)
  // ---------------------------------------------------------------------------
  simd_registers #(.SIMD_WIDTH(MAX_SIMD_WIDTH), .NUM_REGS(8), .DATA_WIDTH(DATA_WIDTH)) simd_regs_out (
    .clk(clk), .rst_n(rst_n), .wr_reg_sel(3'h0), .wr_en(simd_valid),
    .wr_data(simd_pixel_out), .rd_reg_sel_a(), .rd_reg_sel_b(),
    .rd_data_a(), .rd_data_b()
  );

  // ---------------------------------------------------------------------------
  // SIMD Interpolators (generate)
  // ---------------------------------------------------------------------------
  genvar gi;
  generate
    for (gi = 0; gi < MAX_SIMD_WIDTH; gi++) begin : simd_interps
      bilinear_interpolator interp_i (
        .clk(clk), .rst_n(rst_n), .valid_in(interpolate_valid_in),
        .pixel_tl(simd_pixels[0][gi*DATA_WIDTH +: DATA_WIDTH]),
        .pixel_tr(simd_pixels[1][gi*DATA_WIDTH +: DATA_WIDTH]),
        .pixel_bl(simd_pixels[2][gi*DATA_WIDTH +: DATA_WIDTH]),
        .pixel_br(simd_pixels[3][gi*DATA_WIDTH +: DATA_WIDTH]),
        .weight_x(weight_x), .weight_y(weight_y),
        .valid_out(interp_valid[gi]),
        .pixel_out(simd_pixel_out[gi*DATA_WIDTH +: DATA_WIDTH])
      );
    end
  endgenerate

  // ---------------------------------------------------------------------------
  // FSM de Fetch Mejorada
  // ---------------------------------------------------------------------------
  logic fetch_trigger;
  // Trigger para iniciar fetch: cuando addr_gen_valid está alto y estamos en IDLE
  assign fetch_trigger = (addr_gen_valid && fetch_state == FETCH_IDLE);

  // reset inicial
  always_ff @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
      fetch_state <= FETCH_IDLE;
      for (int i = 0; i < 4; i++) simd_pixels[i] <= '0;
      interpolate_valid_in <= 1'b0;
      src_mem_rd_addr <= {ADDR_WIDTH{1'b0}};
    end else begin
      interpolate_valid_in <= 1'b0;

      case (fetch_state)
        FETCH_IDLE: begin
          if (fetch_trigger) begin
            src_mem_rd_addr <= addr_tl;
            fetch_state <= FETCH_TL;
          end
        end

        FETCH_TL: begin
          // Insertar nuevo byte como MSB del bloque (desplazamiento hacia abajo)
          simd_pixels[0] <= {src_mem_rd_data, simd_pixels[0][(MAX_SIMD_WIDTH*DATA_WIDTH-1):DATA_WIDTH]};
          src_mem_rd_addr <= addr_tr;
          fetch_state <= FETCH_TR;
        end

        FETCH_TR: begin
          simd_pixels[1] <= {src_mem_rd_data, simd_pixels[1][(MAX_SIMD_WIDTH*DATA_WIDTH-1):DATA_WIDTH]};
          src_mem_rd_addr <= addr_bl;
          fetch_state <= FETCH_BL;
        end

        FETCH_BL: begin
          simd_pixels[2] <= {src_mem_rd_data, simd_pixels[2][(MAX_SIMD_WIDTH*DATA_WIDTH-1):DATA_WIDTH]};
          src_mem_rd_addr <= addr_br;
          fetch_state <= FETCH_BR;
        end

        FETCH_BR: begin
          simd_pixels[3] <= {src_mem_rd_data, simd_pixels[3][(MAX_SIMD_WIDTH*DATA_WIDTH-1):DATA_WIDTH]};
          interpolate_valid_in <= 1'b1;
          fetch_state <= FETCH_DONE;
        end

        FETCH_DONE: begin
          // Esperar a que el resultado sea consumido (result_write_en)
          if (result_write_en) begin
            fetch_state <= FETCH_IDLE;
          end
        end

        default: fetch_state <= FETCH_IDLE;
      endcase
    end
  end

  // ---------------------------------------------------------------------------
  // Result Write Address Counter
  // ---------------------------------------------------------------------------
  logic [2:0] effective_simd_width;
  assign effective_simd_width = (simd_width == 3'd0) ? 3'd1 : simd_width;

  // dst_w calculado combinacionalmente (evita declarar dentro de always_ff)
  logic [9:0] dst_w;
  always_comb begin
    // scale_factor es fixed_point_t; asumimos que desplazar >>8 da el factor entero
    // Si tu fixed_point tiene otra convención, ajusta esta línea.
    dst_w = (img_width * (scale_factor >> 8));
  end

  always_ff @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
      result_write_addr <= {ADDR_WIDTH{1'b0}};
      pixel_counter_x <= 10'h0;
      pixel_counter_y <= 10'h0;
    end else if (start) begin
      result_write_addr <= {ADDR_WIDTH{1'b0}};
      pixel_counter_x <= 10'h0;
      pixel_counter_y <= 10'h0;
    end else if (result_write_en && interpolate_valid_out) begin
      // sumar effective_simd_width al addr destino, extendiendo a ADDR_WIDTH
      result_write_addr <= result_write_addr + {{(ADDR_WIDTH-3){1'b0}}, effective_simd_width};

      if (pixel_counter_x + effective_simd_width >= dst_w) begin
        pixel_counter_x <= 10'h0;
        pixel_counter_y <= pixel_counter_y + 1;
      end else begin
        pixel_counter_x <= pixel_counter_x + effective_simd_width;
      end
    end
  end

  // ---------------------------------------------------------------------------
  // SIMD Valid Logic (Opción A: AND de todas las lanes activas)
  // ---------------------------------------------------------------------------
  logic [2:0] simd_active_count;
  always_comb begin
    simd_active_count = (simd_width == 3'd0) ? 3'd1 : simd_width;

    for (int i = 0; i < MAX_SIMD_WIDTH; i++) begin
      if (i < simd_active_count)
        interp_valid_masked[i] = interp_valid[i];
      else
        interp_valid_masked[i] = 1'b1; // lanes no usadas consideradas "válidas" para la reducción AND
    end
  end

  assign interpolate_valid_out = &interp_valid_masked;

  // ---------------------------------------------------------------------------
  // Output Assignments
  // ---------------------------------------------------------------------------
  assign dst_mem_wr_addr = result_write_addr;
  assign dst_mem_wr_en   = result_write_en & interpolate_valid_out;
  assign dst_mem_wr_data = simd_pixel_out[DATA_WIDTH-1:0]; // primer pixel del vector
  assign simd_valid      = interpolate_valid_out;

  assign processing_active   = busy;
  assign processing_complete = ready & ~busy;

endmodule