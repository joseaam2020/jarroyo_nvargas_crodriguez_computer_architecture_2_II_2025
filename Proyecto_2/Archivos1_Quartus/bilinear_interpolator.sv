import fixed_point_pkg::*;

module bilinear_interpolator (
    input  logic        clk,
    input  logic        rst_n,

    input  logic        valid_in,
    input  logic [7:0]  pixel_tl,
    input  logic [7:0]  pixel_tr,
    input  logic [7:0]  pixel_bl,
    input  logic [7:0]  pixel_br,
    input  fixed_point_t weight_x,
    input  fixed_point_t weight_y,

    output logic        valid_out,
    output logic [7:0]  pixel_out
);

    logic [15:0] pixel_tl_fixed, pixel_tr_fixed;
    logic [15:0] pixel_bl_fixed, pixel_br_fixed;

    fixed_point_t weight_x_inv, weight_y_inv;

    fixed_point_t prod_tl, prod_tr, prod_bl, prod_br;
    fixed_point_t interp_top, interp_bottom, result;

    logic [2:0] valid_pipe;

    always_comb begin
        pixel_tl_fixed = {pixel_tl, 8'h00};
        pixel_tr_fixed = {pixel_tr, 8'h00};
        pixel_bl_fixed = {pixel_bl, 8'h00};
        pixel_br_fixed = {pixel_br, 8'h00};

        weight_x_inv = 16'h0100 - weight_x;
        weight_y_inv = 16'h0100 - weight_y;
    end

    always_ff @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            prod_tl <= 16'h0;
            prod_tr <= 16'h0;
            prod_bl <= 16'h0;
            prod_br <= 16'h0;
            interp_top <= 16'h0;
            interp_bottom <= 16'h0;
            result <= 16'h0;
            valid_pipe <= 3'b0;
        end else begin
            prod_tl <= fixed_mult(pixel_tl_fixed, fixed_mult(weight_x_inv, weight_y_inv));
            prod_tr <= fixed_mult(pixel_tr_fixed, fixed_mult(weight_x,     weight_y_inv));
            prod_bl <= fixed_mult(pixel_bl_fixed, fixed_mult(weight_x_inv, weight_y));
            prod_br <= fixed_mult(pixel_br_fixed, fixed_mult(weight_x,     weight_y));

            interp_top    <= fixed_add(prod_tl, prod_tr);
            interp_bottom <= fixed_add(prod_bl, prod_br);

            result <= fixed_add(interp_top, interp_bottom);

            valid_pipe <= {valid_pipe[1:0], valid_in};
        end
    end

    assign pixel_out = result[15:8];
    assign valid_out = valid_pipe[2];

endmodule
