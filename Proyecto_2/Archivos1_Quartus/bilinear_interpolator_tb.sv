import fixed_point_pkg::*;

module tb_bilinear_interpolator;

    logic clk, rst_n;
    logic valid_in;
    logic [7:0] pixel_tl, pixel_tr, pixel_bl, pixel_br;
    fixed_point_t weight_x, weight_y;
    logic valid_out;
    logic [7:0] pixel_out;

    bilinear_interpolator dut (
        .clk(clk),
        .rst_n(rst_n),
        .valid_in(valid_in),
        .pixel_tl(pixel_tl),
        .pixel_tr(pixel_tr),
        .pixel_bl(pixel_bl),
        .pixel_br(pixel_br),
        .weight_x(weight_x),
        .weight_y(weight_y),
        .valid_out(valid_out),
        .pixel_out(pixel_out)
    );

    // -------------------------
    // Clock
    // -------------------------
    initial begin
        clk = 1'b0;
        forever #5 clk = ~clk;
    end

    // -------------------------
    // Task sin reales
    // -------------------------
    task apply_test(
        input string name,
        input [7:0] tl, tr, bl, br,
        input fixed_point_t wx, wy,
        input [7:0] expected
    );
        @(negedge clk);
        pixel_tl = tl;
        pixel_tr = tr;
        pixel_bl = bl;
        pixel_br = br;
        weight_x = wx;
        weight_y = wy;
        valid_in = 1'b1;

        @(negedge clk);
        valid_in = 1'b0;

        repeat(3) @(negedge clk);

        $display("Test: %s", name);
        $display("  Output: %0d (expected: %0d)", pixel_out, expected);

        if(pixel_out == expected)
            $display("  PASS\n");
        else
            $display("  FAIL (error: %0d)\n",
                $signed(pixel_out) - $signed(expected));
    endtask

    // -------------------------
    // Test cases
    // -------------------------
    initial begin
        $display("=== Bilinear Interpolator Testbench ===\n");

        rst_n = 1'b0;
        valid_in = 1'b0;
        pixel_tl = 0;
        pixel_tr = 0;
        pixel_bl = 0;
        pixel_br = 0;
        weight_x = 0;
        weight_y = 0;

        #20 rst_n = 1'b1;
        #20;

        // Q8.8 valores:
        // 0.0  = 16'h0000
        // 0.25 = 16'h0040
        // 0.5  = 16'h0080

        apply_test("All pixels equal",
            8'd100, 8'd100, 8'd100, 8'd100,
            16'h0080, 16'h0080,
            8'd100
        );

        apply_test("No interpolation (0,0)",
            8'd50, 8'd100, 8'd100, 8'd100,
            16'h0000, 16'h0000,
            8'd50
        );

        apply_test("Horizontal interpolation",
            8'd50, 8'd100, 8'd50, 8'd100,
            16'h0080, 16'h0000,
            8'd75
        );

        apply_test("Vertical interpolation",
            8'd50, 8'd50, 8'd100, 8'd100,
            16'h0000, 16'h0080,
            8'd75
        );

        apply_test("Full bilinear (center)",
            8'd0, 8'd100, 8'd100, 8'd200,
            16'h0080, 16'h0080,
            8'd100
        );

        apply_test("Quarter interpolation",
            8'd100, 8'd100, 8'd100, 8'd100,
            16'h0040, 16'h0040,
            8'd100
        );

        #20;
        $display("=== All Tests Complete ===\n");
        $finish;
    end

endmodule
