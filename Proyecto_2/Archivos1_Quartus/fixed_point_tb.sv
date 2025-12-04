`timescale 1ns/1ps
import fixed_point_pkg::*;

module fixed_point_tb;

    // Declaraciones obligatorias fuera del initial
    fixed_point_t a1, b1, s1;
    fixed_point_t m1, m2, m3, m4, m5;
    fixed_point_t half, two, neg1, big;
    fixed_point_t s2;   // ❗ ESTE FALTABA

    initial begin
        $display("\n==============================");
        $display("  TESTBENCH fixed_point_pkg");
        $display("==============================\n");

        // -------- Prueba 1 --------
        a1 = 16'h0100; // 1.0
        b1 = 16'h0100; // 1.0
        s1 = fixed_add(a1, b1);
        $display("Suma: 1.0 + 1.0 = %0d (esperado 512)", s1);

        // -------- Prueba 2 --------
        m1 = fixed_mult(a1, b1);
        $display("Mult: 1.0 * 1.0 = %0d (esperado 256)", m1);

        // -------- Prueba 3 --------
        half = 16'h0080; // 0.5
        m2 = fixed_mult(half, half);
        $display("Mult: 0.5 * 0.5 = %0d (esperado 64)", m2);

        // -------- Prueba 4 --------
        two = 16'h0200; // 2.0
        m3 = fixed_mult(two, half);
        $display("Mult: 2.0 * 0.5 = %0d (esperado 256)", m3);

        // -------- Prueba 5 --------
        neg1 = -16'sh0100; // -1.0
        s2 = fixed_add(neg1, half);
        $display("Suma: -1.0 + 0.5 = %0d (esperado -128)", s2);

        // -------- Prueba 6 --------
        m4 = fixed_mult(neg1, half);
        $display("Mult: -1.0 * 0.5 = %0d (esperado -128)", m4);

        // -------- Prueba 7 --------
        big = 16'hFF00; // 255.0
        m5 = fixed_mult(big, a1);
        $display("Mult: 255.0 * 1.0 = %0d (esperado 65280)", m5);

        $display("\nFIN DEL TEST\n");
        $finish;
    end
endmodule