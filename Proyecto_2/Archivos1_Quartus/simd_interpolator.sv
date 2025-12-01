import fixed_point_pkg::*;

module simd_interpolator #(
    parameter SIMD_WIDTH = 4
)(
    input  logic        clk,
    input  logic        rst_n,

    input  logic        valid_in,
    input  logic [SIMD_WIDTH*8-1:0]  pixels_tl,
    input  logic [SIMD_WIDTH*8-1:0]  pixels_tr,
    input  logic [SIMD_WIDTH*8-1:0]  pixels_bl,
    input  logic [SIMD_WIDTH*8-1:0]  pixels_br,
    input  logic [SIMD_WIDTH*16-1:0] weights_x,
    input  logic [SIMD_WIDTH*16-1:0] weights_y,

    output logic        valid_out,
    output logic [SIMD_WIDTH*8-1:0]  pixels_out
);

    genvar i;
    generate
        for (i = 0; i < SIMD_WIDTH; i++) begin : gen_simd_interp
            logic [7:0] pixel_tl_i, pixel_tr_i, pixel_bl_i, pixel_br_i;
            fixed_point_t weight_x_i, weight_y_i;
            logic [7:0] pixel_out_i;
            logic valid_out_i;

            assign pixel_tl_i = pixels_tl[(i+1)*8-1 -: 8];
            assign pixel_tr_i = pixels_tr[(i+1)*8-1 -: 8];
            assign pixel_bl_i = pixels_bl[(i+1)*8-1 -: 8];
            assign pixel_br_i = pixels_br[(i+1)*8-1 -: 8];
            assign weight_x_i = weights_x[(i+1)*16-1 -: 16];
            assign weight_y_i = weights_y[(i+1)*16-1 -: 16];

            bilinear_interpolator interp_unit (
                .clk(clk),
                .rst_n(rst_n),
                .valid_in(valid_in),
                .pixel_tl(pixel_tl_i),
                .pixel_tr(pixel_tr_i),
                .pixel_bl(pixel_bl_i),
                .pixel_br(pixel_br_i),
                .weight_x(weight_x_i),
                .weight_y(weight_y_i),
                .valid_out(valid_out_i),
                .pixel_out(pixel_out_i)
            );

            assign pixels_out[(i+1)*8-1 -: 8] = pixel_out_i;
        end
    endgenerate

    assign valid_out = gen_simd_interp[0].valid_out_i;

endmodule