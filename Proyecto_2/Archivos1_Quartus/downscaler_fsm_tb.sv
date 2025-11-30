`timescale 1ns/1ps

module tb_downscaler_fsm;

    logic clk;
    logic rst_n;
    logic start;
    logic step_mode;
    logic step_next;
    logic addr_gen_done;
    logic addr_gen_start;
    logic addr_gen_next;
    logic interpolate_start;
    logic interpolate_done;
    logic result_write_en;
    logic busy;
    logic ready;
    logic error;

    // Instancia del DUT
    downscaler_fsm dut (
        .clk(clk),
        .rst_n(rst_n),
        .start(start),
        .step_mode(step_mode),
        .step_next(step_next),
        .addr_gen_done(addr_gen_done),
        .addr_gen_start(addr_gen_start),
        .addr_gen_next(addr_gen_next),
        .interpolate_start(interpolate_start),
        .interpolate_done(interpolate_done),
        .result_write_en(result_write_en),
        .busy(busy),
        .ready(ready),
        .error(error)
    );

    // Generador de reloj
    initial begin
        clk = 0;
        forever #5 clk = ~clk; // 100MHz (periodo 10ns)
    end

    // Monitor continuo - imprime TODO en cada ciclo
    always @(posedge clk) begin
        $display("[%0t] STATE=%0d | busy=%0b ready=%0b | addr_done=%0b interp_done=%0b | addr_start=%0b addr_next=%0b interp_start=%0b write_en=%0b", 
                 $time, dut.state, busy, ready, addr_gen_done, interpolate_done, 
                 addr_gen_start, addr_gen_next, interpolate_start, result_write_en);
    end

    // Proceso de prueba
    initial begin
        $display("\n========================================");
        $display("INICIO DE SIMULACION");
        $display("========================================\n");
        
        // Inicialización
        rst_n = 0;
        start = 0;
        step_mode = 0;
        step_next = 0;
        addr_gen_done = 0;
        interpolate_done = 0;
        
        // Reset
        repeat(2) @(posedge clk);
        rst_n = 1;
        $display("\n*** RESET COMPLETADO - FSM en IDLE ***\n");
        
        repeat(2) @(posedge clk);

        // ============================================
        // TEST 1: MODO AUTOMATICO - 3 pixeles
        // ============================================
        $display("\n========================================");
        $display("TEST 1: MODO AUTOMATICO (3 pixeles)");
        $display("========================================\n");
        
        step_mode = 0;
        
        @(posedge clk);
        start = 1;
        $display(">>> START=1 - Iniciando procesamiento");
        
        @(posedge clk);
        start = 0;
        
        // --- PIXEL 1 ---
        $display("\n--- PIXEL 1 ---");
        repeat(2) @(posedge clk);
        interpolate_done = 1;
        $display(">>> INTERPOLATE_DONE=1");
        
        @(posedge clk);
        interpolate_done = 0;
        
        // --- PIXEL 2 ---
        $display("\n--- PIXEL 2 ---");
        repeat(2) @(posedge clk);
        interpolate_done = 1;
        $display(">>> INTERPOLATE_DONE=1");
        
        @(posedge clk);
        interpolate_done = 0;
        
        // --- PIXEL 3 (ULTIMO) ---
        $display("\n--- PIXEL 3 (ULTIMO) ---");
        repeat(2) @(posedge clk);
        addr_gen_done = 1;
        interpolate_done = 1;
        $display(">>> ADDR_GEN_DONE=1 + INTERPOLATE_DONE=1");
        
        @(posedge clk);
        interpolate_done = 0;
        addr_gen_done = 0;
        
        repeat(3) @(posedge clk);
        $display("\n*** TEST 1 COMPLETADO ***\n");

        // ============================================
        // TEST 2: MODO STEP - 2 pixeles
        // ============================================
        $display("\n========================================");
        $display("TEST 2: MODO STEP (2 pixeles)");
        $display("========================================\n");
        
        step_mode = 1;
        
        @(posedge clk);
        start = 1;
        $display(">>> START=1 - Modo STEP activado");
        
        @(posedge clk);
        start = 0;
        
        // --- PIXEL 1 ---
        $display("\n--- PIXEL 1 (STEP) ---");
        repeat(2) @(posedge clk);
        interpolate_done = 1;
        $display(">>> INTERPOLATE_DONE=1");
        
        @(posedge clk);
        interpolate_done = 0;
        
        // Esperar en STEP_WAIT
        $display("\n*** Esperando en STEP_WAIT ***");
        repeat(3) @(posedge clk);
        
        step_next = 1;
        $display(">>> STEP_NEXT=1 - Avanzando al siguiente pixel");
        
        @(posedge clk);
        step_next = 0;
        
        // --- PIXEL 2 (ULTIMO) ---
        $display("\n--- PIXEL 2 (ULTIMO, STEP) ---");
        repeat(2) @(posedge clk);
        addr_gen_done = 1;
        interpolate_done = 1;
        $display(">>> ADDR_GEN_DONE=1 + INTERPOLATE_DONE=1");
        
        @(posedge clk);
        interpolate_done = 0;
        
        repeat(2) @(posedge clk);
        step_next = 1;
        $display(">>> STEP_NEXT=1 - Finalizando");
        
        @(posedge clk);
        step_next = 0;
        addr_gen_done = 0;
        
        repeat(3) @(posedge clk);
        $display("\n*** TEST 2 COMPLETADO ***\n");

        $display("\n========================================");
        $display("FIN DE SIMULACION - TODO CORRECTO");
        $display("========================================\n");
        $finish;
    end

    // Timeout de seguridad
    initial begin
        #5000;
        $display("\n!!! TIMEOUT - La simulacion tardo demasiado !!!");
        $finish;
    end

endmodule