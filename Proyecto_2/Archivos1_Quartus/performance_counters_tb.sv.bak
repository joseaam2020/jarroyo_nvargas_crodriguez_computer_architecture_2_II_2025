module tb_performance_counters;
    // Señales
    logic        clk;
    logic        rst_n;
    logic        clear;
    logic        flop_inc;
    logic [31:0] flop_count;
    logic        mem_read_inc;
    logic        mem_write_inc;
    logic [31:0] total_flops;
    logic [31:0] total_mem_reads;
    logic [31:0] total_mem_writes;
    
    // Instancia DUT
    performance_counters dut (
        .clk(clk),
        .rst_n(rst_n),
        .clear(clear),
        .flop_inc(flop_inc),
        .flop_count(flop_count),
        .mem_read_inc(mem_read_inc),
        .mem_write_inc(mem_write_inc),
        .total_flops(total_flops),
        .total_mem_reads(total_mem_reads),
        .total_mem_writes(total_mem_writes)
    );
    
    // ===============================================================
    // Clock
    // ===============================================================
    initial begin
        clk = 0;
        forever #5 clk = ~clk;
    end
    
    // ===============================================================
    // Testbench
    // ===============================================================
    initial begin
        $display("\n=== Performance Counters Testbench ===\n");
        
        // Inicialización
        rst_n = 0;
        clear = 0;
        flop_inc = 0;
        flop_count = 32'h0;
        mem_read_inc = 0;
        mem_write_inc = 0;
        
        // Reset
        #15;
        rst_n = 1;
        @(posedge clk);
        @(posedge clk);
        
        // ------------------------------------------------------
        // Test 1: Verify reset values
        // ------------------------------------------------------
        $display("Test 1: Verify reset values");
        $display("  total_flops = %0d (expected: 0)", total_flops);
        $display("  total_mem_reads = %0d (expected: 0)", total_mem_reads);
        $display("  total_mem_writes = %0d (expected: 0)", total_mem_writes);
        if (total_flops == 0 && total_mem_reads == 0 && total_mem_writes == 0)
            $display("  PASS\n");
        else
            $display("  FAIL\n");
        
        // ------------------------------------------------------
        // Test 2: Increment FLOP counter
        // ------------------------------------------------------
        $display("Test 2: Increment FLOP counter");
        flop_count = 32'd100;
        flop_inc = 1;
        @(posedge clk);
        flop_inc = 0;
        @(posedge clk);
        
        $display("  Added 100 FLOPs");
        $display("  total_flops = %0d (expected: 100)", total_flops);
        if (total_flops == 100)
            $display("  PASS\n");
        else
            $display("  FAIL\n");
        
        // ------------------------------------------------------
        // Test 3: Multiple FLOP increments
        // ------------------------------------------------------
        $display("Test 3: Multiple FLOP increments");
        flop_count = 32'd50;
        flop_inc = 1;
        @(posedge clk);
        flop_count = 32'd75;
        @(posedge clk);
        flop_count = 32'd25;
        @(posedge clk);
        flop_inc = 0;
        @(posedge clk);
        
        $display("  Added 50 + 75 + 25 = 150 FLOPs");
        $display("  total_flops = %0d (expected: 250)", total_flops);
        if (total_flops == 250)
            $display("  PASS\n");
        else
            $display("  FAIL\n");
        
        // ------------------------------------------------------
        // Test 4: Increment memory read counter
        // ------------------------------------------------------
        $display("Test 4: Increment memory read counter");
        mem_read_inc = 1;
        @(posedge clk);
        @(posedge clk);
        @(posedge clk);
        mem_read_inc = 0;
        @(posedge clk);
        
        $display("  Performed 3 memory reads");
        $display("  total_mem_reads = %0d (expected: 3)", total_mem_reads);
        if (total_mem_reads == 3)
            $display("  PASS\n");
        else
            $display("  FAIL\n");
        
        // ------------------------------------------------------
        // Test 5: Increment memory write counter
        // ------------------------------------------------------
        $display("Test 5: Increment memory write counter");
        mem_write_inc = 1;
        @(posedge clk);
        @(posedge clk);
        @(posedge clk);
        @(posedge clk);
        @(posedge clk);
        mem_write_inc = 0;
        @(posedge clk);
        
        $display("  Performed 5 memory writes");
        $display("  total_mem_writes = %0d (expected: 5)", total_mem_writes);
        if (total_mem_writes == 5)
            $display("  PASS\n");
        else
            $display("  FAIL\n");
        
        // ------------------------------------------------------
        // Test 6: Simultaneous increments
        // ------------------------------------------------------
        $display("Test 6: Simultaneous increments");
        flop_count = 32'd200;
        flop_inc = 1;
        mem_read_inc = 1;
        mem_write_inc = 1;
        @(posedge clk);
        flop_inc = 0;
        mem_read_inc = 0;
        mem_write_inc = 0;
        @(posedge clk);
        
        $display("  Incremented all counters simultaneously");
        $display("  total_flops = %0d (expected: 450)", total_flops);
        $display("  total_mem_reads = %0d (expected: 4)", total_mem_reads);
        $display("  total_mem_writes = %0d (expected: 6)", total_mem_writes);
        if (total_flops == 450 && total_mem_reads == 4 && total_mem_writes == 6)
            $display("  PASS\n");
        else
            $display("  FAIL\n");
        
        // ------------------------------------------------------
        // Test 7: Clear counters
        // ------------------------------------------------------
        $display("Test 7: Clear counters");
        clear = 1;
        @(posedge clk);
        clear = 0;
        @(posedge clk);
        
        $display("  Cleared all counters");
        $display("  total_flops = %0d (expected: 0)", total_flops);
        $display("  total_mem_reads = %0d (expected: 0)", total_mem_reads);
        $display("  total_mem_writes = %0d (expected: 0)", total_mem_writes);
        if (total_flops == 0 && total_mem_reads == 0 && total_mem_writes == 0)
            $display("  PASS\n");
        else
            $display("  FAIL\n");
        
        // ------------------------------------------------------
        // Test 8: Increment after clear
        // ------------------------------------------------------
        $display("Test 8: Increment after clear");
        flop_count = 32'd1000;
        flop_inc = 1;
        mem_read_inc = 1;
        @(posedge clk);
        flop_inc = 0;
        mem_read_inc = 0;
        @(posedge clk);
        
        $display("  Added 1000 FLOPs and 1 read after clear");
        $display("  total_flops = %0d (expected: 1000)", total_flops);
        $display("  total_mem_reads = %0d (expected: 1)", total_mem_reads);
        if (total_flops == 1000 && total_mem_reads == 1)
            $display("  PASS\n");
        else
            $display("  FAIL\n");
        
        // ------------------------------------------------------
        // Test 9: Reset during operation
        // ------------------------------------------------------
        $display("Test 9: Reset during operation");
        rst_n = 0;
        @(posedge clk);
        @(posedge clk);
        rst_n = 1;
        @(posedge clk);
        
        $display("  Applied reset during operation");
        $display("  total_flops = %0d (expected: 0)", total_flops);
        $display("  total_mem_reads = %0d (expected: 0)", total_mem_reads);
        $display("  total_mem_writes = %0d (expected: 0)", total_mem_writes);
        if (total_flops == 0 && total_mem_reads == 0 && total_mem_writes == 0)
            $display("  PASS\n");
        else
            $display("  FAIL\n");
        
        #20;
        $display("=== All Performance Counter Tests Complete ===\n");
        $finish;
    end
endmodule