// bilinear_interpolator.sv
// Interpolador bilineal single-lane para usar en el módulo SIMD
// Realiza interpolación bilineal: resultado = f(tl, tr, bl, br, weight_x, weight_y)

`timescale 1ns/1ps
import fixed_point_pkg::*;

module bilinear_interpolator (
  input  logic                clk,
  input  logic                rst_n,
  input  logic                valid_in,

  // 4 píxeles vecinos
  input  logic [7:0]          pixel_tl,  // top-left
  input  logic [7:0]          pixel_tr,  // top-right
  input  logic [7:0]          pixel_bl,  // bottom-left
  input  logic [7:0]          pixel_br,  // bottom-right

  // Pesos de interpolación (formato fixed-point 8.8)
  input  fixed_point_t        weight_x,  // peso horizontal (0.0 a 1.0)
  input  fixed_point_t        weight_y,  // peso vertical (0.0 a 1.0)

  // Salida
  output logic                valid_out,
  output logic [7:0]          pixel_out
);

  // =====================================================================
  // Pipeline de interpolación bilineal (latencia = 4 ciclos)
  // =====================================================================
  
  // Stage 0: Entrada (registros)
  logic [7:0] tl_s0, tr_s0, bl_s0, br_s0;
  fixed_point_t wx_s0, wy_s0;
  logic valid_s0;

  always_ff @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
      tl_s0 <= 8'h00;
      tr_s0 <= 8'h00;
      bl_s0 <= 8'h00;
      br_s0 <= 8'h00;
      wx_s0 <= 16'h0000;
      wy_s0 <= 16'h0000;
      valid_s0 <= 1'b0;
    end else begin
      tl_s0 <= pixel_tl;
      tr_s0 <= pixel_tr;
      bl_s0 <= pixel_bl;
      br_s0 <= pixel_br;
      wx_s0 <= weight_x;
      wy_s0 <= weight_y;
      valid_s0 <= valid_in;
    end
  end

  // Stage 1: Interpolación horizontal (top e inferior)
  // top_lerp = tl + wx * (tr - tl)
  // bot_lerp = bl + wx * (br - bl)
  logic [15:0] top_lerp_s1, bot_lerp_s1;
  logic valid_s1;

  always_ff @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
      top_lerp_s1 <= 16'h0000;
      bot_lerp_s1 <= 16'h0000;
      valid_s1 <= 1'b0;
    end else begin
      // top_lerp = tl + (wx * (tr - tl)) >> 8
      top_lerp_s1 <= {8'h00, tl_s0} + fixed_mult(wx_s0, {8'h00, (tr_s0 - tl_s0)});
      
      // bot_lerp = bl + (wx * (br - bl)) >> 8
      bot_lerp_s1 <= {8'h00, bl_s0} + fixed_mult(wx_s0, {8'h00, (br_s0 - bl_s0)});
      
      valid_s1 <= valid_s0;
    end
  end

  // Stage 2: Saturación después de interpolación horizontal
  logic [7:0] top_sat_s2, bot_sat_s2;
  logic valid_s2;

  always_ff @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
      top_sat_s2 <= 8'h00;
      bot_sat_s2 <= 8'h00;
      valid_s2 <= 1'b0;
    end else begin
      // Saturar a 8 bits
      top_sat_s2 <= (top_lerp_s1[15:8] > 8'hFF) ? 8'hFF : top_lerp_s1[7:0];
      bot_sat_s2 <= (bot_lerp_s1[15:8] > 8'hFF) ? 8'hFF : bot_lerp_s1[7:0];
      valid_s2 <= valid_s1;
    end
  end

  // Stage 3: Interpolación vertical
  // resultado = top_sat + wy * (bot_sat - top_sat)
  logic [15:0] final_lerp_s3;
  logic valid_s3;

  always_ff @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
      final_lerp_s3 <= 16'h0000;
      valid_s3 <= 1'b0;
    end else begin
      // final = top_sat + (wy * (bot_sat - top_sat)) >> 8
      final_lerp_s3 <= {8'h00, top_sat_s2} + fixed_mult(wy_s0, {8'h00, (bot_sat_s2 - top_sat_s2)});
      valid_s3 <= valid_s2;
    end
  end

  // Stage 4: Saturación final
  always_ff @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
      pixel_out <= 8'h00;
      valid_out <= 1'b0;
    end else begin
      // Saturar resultado final a 8 bits
      pixel_out <= (final_lerp_s3[15:8] > 8'hFF) ? 8'hFF : final_lerp_s3[7:0];
      valid_out <= valid_s3;
    end
  end

  // =====================================================================
  // Función auxiliar para multiplicación fixed-point
  // =====================================================================
  function automatic logic [15:0] fixed_mult(
    input logic [15:0] a,
    input logic [15:0] b
  );
    logic [31:0] product;
    product = a * b;
    return product[23:8];  // Tomar los 16 bits del medio (desplazar 8 bits)
  endfunction

endmodule
