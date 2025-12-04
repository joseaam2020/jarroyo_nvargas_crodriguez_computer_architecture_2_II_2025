import fixed_point_pkg::*;
module bilinear_interpolator_simd #(
  parameter int SIMD_WIDTH = 8
)(
  input  logic                     clk, 
  input  logic                     rst_n,
  input  logic                     valid_in,
  // 4 vecinos por lane (empaquetados)
  input  logic [SIMD_WIDTH-1:0][7:0] pixel_tl,
  input  logic [SIMD_WIDTH-1:0][7:0] pixel_tr,
  input  logic [SIMD_WIDTH-1:0][7:0] pixel_bl,
  input  logic [SIMD_WIDTH-1:0][7:0] pixel_br,
  // pesos por lane
  input  fixed_point_t weight_x   [SIMD_WIDTH],
  input  fixed_point_t weight_y   [SIMD_WIDTH],
  // resultados por lane
  output logic                     valid_out,
  output logic [SIMD_WIDTH-1:0][7:0] pixel_out
);
  // ================================
  // Pipeline del valid (latencia = 4)
  // ================================
  logic [3:0] valid_pipe;
  always_ff @(posedge clk or negedge rst_n) begin
    if (!rst_n)
      valid_pipe <= 4'b0;
    else
      valid_pipe <= { valid_pipe[2:0], valid_in };
  end
  assign valid_out = valid_pipe[3];
  
  // ================================
  // Instanciamos SIMD_WIDTH lanes
  // ================================
  genvar i;  // ← Declarar aquí, FUERA del for
  generate
    for (i = 0; i < SIMD_WIDTH; i++) begin : lanes
      bilinear_interpolator interp_lane (
        .clk(clk),
        .rst_n(rst_n),
        .valid_in(valid_in),
        .pixel_tl(pixel_tl[i]),
        .pixel_tr(pixel_tr[i]),
        .pixel_bl(pixel_bl[i]),
        .pixel_br(pixel_br[i]),
        .weight_x(weight_x[i]),
        .weight_y(weight_y[i]),
        .valid_out(),         // manejado globalmente
        .pixel_out(pixel_out[i])
      );
    end
  endgenerate
endmodule