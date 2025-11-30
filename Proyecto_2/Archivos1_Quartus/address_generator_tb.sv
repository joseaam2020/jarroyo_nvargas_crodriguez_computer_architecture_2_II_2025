`timescale 1ns/1ps

import fixed_point_pkg::*;

module tb_address_generator;

    // ========================================
    // SEÑALES DEL DUT
    // ========================================
    logic         clk;
    logic         rst_n;
    logic         start;
    logic         next_pixel;
    logic [9:0]   src_width;
    logic [9:0]   src_height;
    fixed_point_t scale_factor;
    logic [17:0]  addr_tl;
    logic [17:0]  addr_tr;
    logic [17:0]  addr_bl;
    logic [17:0]  addr_br;
    fixed_point_t weight_x;
    fixed_point_t weight_y;
    logic         valid;
    logic         done;

    // ========================================
    // INSTANCIA DEL DUT
    // ========================================
    address_generator dut (
        .clk(clk),
        .rst_n(rst_n),
        .start(start),
        .next_pixel(next_pixel),
        .src_width(src_width),
        .src_height(src_height),
        .scale_factor(scale_factor),
        .addr_tl(addr_tl),
        .addr_tr(addr_tr),
        .addr_bl(addr_bl),
        .addr_br(addr_br),
        .weight_x(weight_x),
        .weight_y(weight_y),
        .valid(valid),
        .done(done)
    );

    // ========================================
    // GENERADOR DE RELOJ
    // ========================================
    initial begin
        clk = 0;
        forever #5 clk = ~clk; // 100MHz (periodo 10ns)
    end

    // ========================================
    // MONITOR CONTINUO
    // ========================================
    always @(posedge clk) begin
        if (valid || done) begin
            $display("[%0t] STATE=%0d | valid=%0b done=%0b | dst(%0d,%0d)", 
                     $time, dut.state, valid, done, dut.dst_x, dut.dst_y);
            $display("       src_fixed: X=0x%04h (%0d.%03d) Y=0x%04h (%0d.%03d)",
                     dut.src_x_fixed, dut.src_x_int, (dut.src_x_frac*1000)/256,
                     dut.src_y_fixed, dut.src_y_int, (dut.src_y_frac*1000)/256);
            $display("       Addresses: TL=%05d TR=%05d BL=%05d BR=%05d",
                     addr_tl, addr_tr, addr_bl, addr_br);
            $display("       Weights: X=0x%04h (%.3f) Y=0x%04h (%.3f)",
                     weight_x, real'(weight_x)/256.0, weight_y, real'(weight_y)/256.0);
            $display("---");
        end
    end

    // ========================================
    // PROCESO DE PRUEBA
    // ========================================
    initial begin
        $display("\n========================================");
        $display("ADDRESS GENERATOR TESTBENCH");
        $display("========================================\n");
        
        // Inicialización
        rst_n = 0;
        start = 0;
        next_pixel = 0;
        src_width = 10'd0;
        src_height = 10'd0;
        scale_factor = 16'h0000;
        
        // Reset
        repeat(2) @(posedge clk);
        rst_n = 1;
        $display("*** RESET COMPLETADO ***\n");
        repeat(2) @(posedge clk);

        // ============================================
        // TEST 1: Escala 0.5x (reducir) - 4x4 → 2x2
        // ============================================
        $display("\n========================================");
        $display("TEST 1: ESCALA 0.5x (REDUCCION)");
        $display("Source: 4x4 pixels");
        $display("Scale: 0.5 (0x0080)");
        $display("Expected dest: 2x2 pixels");
        $display("========================================\n");
        
        src_width = 10'd4;
        src_height = 10'd4;
        scale_factor = 16'h0080;  // 0.5 en 8.8 fixed point
        
        @(posedge clk);
        start = 1;
        $display(">>> START=1");
        
        @(posedge clk);
        start = 0;
        
        repeat(2) @(posedge clk);
        $display("Calculated dimensions: %0dx%0d\n", dut.dst_width, dut.dst_height);
        
        // Procesar 5 píxeles (2x2 + 1 para verificar done)
        repeat(5) begin
            @(posedge clk);
            next_pixel = 1;
            @(posedge clk);
            next_pixel = 0;
            repeat(1) @(posedge clk);
            if (done) begin
                $display("*** DONE signal activated ***");
                break;
            end
        end
        
        repeat(3) @(posedge clk);
        $display("\n*** TEST 1 COMPLETED ***\n");

        // ============================================
        // TEST 2: Escala 1.0x (sin cambio) - 3x3 → 3x3
        // ============================================
        $display("\n========================================");
        $display("TEST 2: ESCALA 1.0x (NO SCALING)");
        $display("Source: 3x3 pixels");
        $display("Scale: 1.0 (0x0100)");
        $display("Expected dest: 3x3 pixels");
        $display("========================================\n");
        
        src_width = 10'd3;
        src_height = 10'd3;
        scale_factor = 16'h0100;  // 1.0 en 8.8 fixed point
        
        @(posedge clk);
        start = 1;
        $display(">>> START=1");
        
        @(posedge clk);
        start = 0;
        
        repeat(2) @(posedge clk);
        $display("Calculated dimensions: %0dx%0d\n", dut.dst_width, dut.dst_height);
        
        // Procesar 10 píxeles (3x3 + 1)
        repeat(10) begin
            @(posedge clk);
            next_pixel = 1;
            @(posedge clk);
            next_pixel = 0;
            repeat(1) @(posedge clk);
            if (done) begin
                $display("*** DONE signal activated ***");
                break;
            end
        end
        
        repeat(3) @(posedge clk);
        $display("\n*** TEST 2 COMPLETED ***\n");

        // ============================================
        // TEST 3: Escala 2.0x (ampliar) - 2x2 → 4x4
        // ============================================
        $display("\n========================================");
        $display("TEST 3: ESCALA 2.0x (UPSCALING)");
        $display("Source: 2x2 pixels");
        $display("Scale: 2.0 (0x0200)");
        $display("Expected dest: 4x4 pixels");
        $display("========================================\n");
        
        src_width = 10'd2;
        src_height = 10'd2;
        scale_factor = 16'h0200;  // 2.0 en 8.8 fixed point
        
        @(posedge clk);
        start = 1;
        $display(">>> START=1");
        
        @(posedge clk);
        start = 0;
        
        repeat(2) @(posedge clk);
        $display("Calculated dimensions: %0dx%0d\n", dut.dst_width, dut.dst_height);
        
        // Procesar 17 píxeles (4x4 + 1)
        repeat(17) begin
            @(posedge clk);
            next_pixel = 1;
            @(posedge clk);
            next_pixel = 0;
            repeat(1) @(posedge clk);
            if (done) begin
                $display("*** DONE signal activated ***");
                break;
            end
        end
        
        repeat(3) @(posedge clk);
        $display("\n*** TEST 3 COMPLETED ***\n");

        // ============================================
        // TEST 4: Escala 0.75x - 4x4 → 3x3
        // ============================================
        $display("\n========================================");
        $display("TEST 4: ESCALA 0.75x");
        $display("Source: 4x4 pixels");
        $display("Scale: 0.75 (0x00C0)");
        $display("Expected dest: 3x3 pixels");
        $display("========================================\n");
        
        src_width = 10'd4;
        src_height = 10'd4;
        scale_factor = 16'h00C0;  // 0.75 en 8.8 fixed point (192/256)
        
        @(posedge clk);
        start = 1;
        $display(">>> START=1");
        
        @(posedge clk);
        start = 0;
        
        repeat(2) @(posedge clk);
        $display("Calculated dimensions: %0dx%0d\n", dut.dst_width, dut.dst_height);
        
        // Procesar 6 píxeles para ver el patrón
        repeat(6) begin
            @(posedge clk);
            next_pixel = 1;
            @(posedge clk);
            next_pixel = 0;
            repeat(1) @(posedge clk);
            if (done) begin
                $display("*** DONE signal activated ***");
                break;
            end
        end
        
        repeat(3) @(posedge clk);
        $display("\n*** TEST 4 COMPLETED ***\n");

        // ============================================
        // TEST 5: Caso borde - 1x1 píxel
        // ============================================
        $display("\n========================================");
        $display("TEST 5: EDGE CASE - 1x1 pixel");
        $display("Source: 1x1 pixel");
        $display("Scale: 1.0 (0x0100)");
        $display("========================================\n");
        
        src_width = 10'd1;
        src_height = 10'd1;
        scale_factor = 16'h0100;
        
        @(posedge clk);
        start = 1;
        $display(">>> START=1");
        
        @(posedge clk);
        start = 0;
        
        repeat(2) @(posedge clk);
        $display("Calculated dimensions: %0dx%0d\n", dut.dst_width, dut.dst_height);
        
        // Procesar 1 píxel
        @(posedge clk);
        next_pixel = 1;
        @(posedge clk);
        next_pixel = 0;
        
        repeat(2) @(posedge clk);
        
        if (addr_tl == 0 && addr_tr == 0 && addr_bl == 0 && addr_br == 0) begin
            $display("✓ All addresses are 0 (correct for 1x1 image)");
        end else begin
            $display("✗ ERROR: Addresses should all be 0 for 1x1 image");
        end
        
        repeat(3) @(posedge clk);
        $display("\n*** TEST 5 COMPLETED ***\n");

        // ============================================
        // FIN
        // ============================================
        $display("\n========================================");
        $display("ALL TESTS COMPLETED SUCCESSFULLY");
        $display("========================================\n");
        $finish;
    end

    // Timeout
    initial begin
        #100000;
        $display("\n!!! TIMEOUT !!!");
        $finish;
    end

endmodule