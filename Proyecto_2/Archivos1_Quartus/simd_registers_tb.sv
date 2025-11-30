module tb_simd_registers;
    // Parámetros
    parameter SIMD_WIDTH = 4;
    parameter NUM_REGS   = 8;
    
    // Señales
    logic clk;
    logic rst_n;
    logic [2:0]  wr_reg_sel;
    logic        wr_en;
    logic [SIMD_WIDTH*8-1:0] wr_data;
    logic [2:0]  rd_reg_sel_a;
    logic [2:0]  rd_reg_sel_b;
    logic [SIMD_WIDTH*8-1:0] rd_data_a;
    logic [SIMD_WIDTH*8-1:0] rd_data_b;
    
    // Variables auxiliares para valores esperados
    logic [SIMD_WIDTH*8-1:0] expected_a;
    logic [SIMD_WIDTH*8-1:0] expected_b;
    logic [7:0] byte_val;
    
    // Instancia DUT
    simd_registers #(
        .SIMD_WIDTH(SIMD_WIDTH),
        .NUM_REGS(NUM_REGS)
    ) dut (
        .clk(clk),
        .rst_n(rst_n),
        .wr_reg_sel(wr_reg_sel),
        .wr_en(wr_en),
        .wr_data(wr_data),
        .rd_reg_sel_a(rd_reg_sel_a),
        .rd_reg_sel_b(rd_reg_sel_b),
        .rd_data_a(rd_data_a),
        .rd_data_b(rd_data_b)
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
        $display("\n=== SIMD Registers Testbench ===\n");
        
        // Reset
        rst_n = 0;
        wr_en = 0;
        wr_reg_sel = 0;
        wr_data = '0;
        rd_reg_sel_a = 0;
        rd_reg_sel_b = 1;
        #15;
        rst_n = 1;
        @(posedge clk);
        
        // ------------------------------------------------------
        // Test 1: Write a single register
        // ------------------------------------------------------
        $display("Test 1: Write and read a single register");
        wr_reg_sel = 3; 
        wr_data = 32'hDEADBEEF;
        wr_en = 1;
        @(posedge clk);
        wr_en = 0;
        @(posedge clk);
        
        rd_reg_sel_a = 3;
        rd_reg_sel_b = 0;
        #1;
        $display("  rd_data_a = 0x%h (expected: 0xDEADBEEF)", rd_data_a);
        $display("  rd_data_b = 0x%h (expected: 0x00000000)", rd_data_b);
        if (rd_data_a == 32'hDEADBEEF && rd_data_b == 32'h0)
            $display("  PASS\n");
        else
            $display("  FAIL\n");
        
        // ------------------------------------------------------
        // Test 2: Write multiple registers
        // ------------------------------------------------------
        $display("Test 2: Write multiple registers");
        for (int i = 0; i < NUM_REGS; i++) begin
            wr_reg_sel = i;
            byte_val = 8'h10 + i;
            // Escribir explícitamente cada byte
            wr_data[31:24] = byte_val;
            wr_data[23:16] = byte_val;
            wr_data[15:8]  = byte_val;
            wr_data[7:0]   = byte_val;
            wr_en = 1;
            $display("  Writing to reg[%0d]: wr_data = 0x%h", i, wr_data);
            @(posedge clk);
        end
        wr_en = 0;
        @(posedge clk);
        
        $display("  Reading back registers:");
        for (int i = 0; i < NUM_REGS; i++) begin
            rd_reg_sel_a = i;
            #1;
            byte_val = 8'h10 + i;
            expected_a[31:24] = byte_val;
            expected_a[23:16] = byte_val;
            expected_a[15:8]  = byte_val;
            expected_a[7:0]   = byte_val;
            $display("    regs[%0d] = 0x%h (expected: 0x%h)", i, rd_data_a, expected_a);
            if (rd_data_a == expected_a)
                $display("      PASS");
            else
                $display("      FAIL");
        end
        $display("");
        
        // ------------------------------------------------------
        // Test 3: Check register 7 (maximum valid index)
        // ------------------------------------------------------
        $display("Test 3: Read maximum valid register index");
        rd_reg_sel_a = 3'd7;
        #1;
        expected_a = 32'h17171717;
        $display("  Reading reg 7: 0x%h (expected: 0x%h)", rd_data_a, expected_a);
        if (rd_data_a == expected_a)
            $display("  PASS\n");
        else
            $display("  FAIL\n");
        
        // ------------------------------------------------------
        // Test 4: Read registers within valid range
        // ------------------------------------------------------
        $display("Test 4: Read registers within valid range (dual port)");
        for (int i = 0; i < NUM_REGS; i++) begin
            rd_reg_sel_a = i;
            rd_reg_sel_b = (i + 1) % NUM_REGS;
            #1;
            
            byte_val = 8'h10 + i;
            expected_a[31:24] = byte_val;
            expected_a[23:16] = byte_val;
            expected_a[15:8]  = byte_val;
            expected_a[7:0]   = byte_val;
            
            byte_val = 8'h10 + ((i+1) % NUM_REGS);
            expected_b[31:24] = byte_val;
            expected_b[23:16] = byte_val;
            expected_b[15:8]  = byte_val;
            expected_b[7:0]   = byte_val;
            
            $display("  rd_data_a (regs[%0d]) = 0x%h, expected = 0x%h", 
                     i, rd_data_a, expected_a);
            $display("  rd_data_b (regs[%0d]) = 0x%h, expected = 0x%h", 
                     (i+1) % NUM_REGS, rd_data_b, expected_b);
            
            if (rd_data_a == expected_a && rd_data_b == expected_b)
                $display("  PASS\n");
            else
                $display("  FAIL\n");
        end
        
        $display("=== All SIMD Register Tests Complete ===\n");
        $finish;
    end
endmodule