module tb_image_memory;
    logic clk;
    logic [17:0] rd_addr, wr_addr;
    logic [7:0] rd_data, wr_data;
    logic wr_en;
    
    image_memory dut (
        .clk(clk),
        .rd_addr(rd_addr),
        .rd_data(rd_data),
        .wr_addr(wr_addr),
        .wr_en(wr_en),
        .wr_data(wr_data)
    );
    
    // ==========================
    // CLOCK
    // ==========================
    initial begin
        clk = 1'b0;
        forever #5 clk = ~clk;
    end
    
    // ==========================
    // TESTBENCH
    // ==========================
    initial begin
        $display("=== Image Memory Testbench ===\n");
        
        // Reset/Inicialización
        wr_en = 1'b0;
        rd_addr = 18'h0;
        wr_addr = 18'h0;
        wr_data = 8'h0;
        #20;
        
        // --------------------------
        // TEST 1: Write and read single pixel
        // --------------------------
        $display("Test 1: Write and read single pixel");
        wr_addr = 18'h0;
        wr_data = 8'hAB;
        wr_en = 1'b1;
        @(posedge clk);
        wr_en = 1'b0;
        @(posedge clk);
        
        rd_addr = 18'h0;
        @(posedge clk);
        @(posedge clk); // Segundo ciclo para que rd_data se estabilice
        $display("  Write 0xAB to addr 0x0");
        $display("  Read from addr 0x0: 0x%h (expected: 0xAB)", rd_data);
        if (rd_data == 8'hAB) $display("  PASS\n");
        else $display("  FAIL\n");
        
        // --------------------------
        // TEST 2: Write multiple pixels
        // --------------------------
        $display("Test 2: Write multiple pixels");
        for (int i = 0; i < 10; i++) begin
            wr_addr = i[17:0];
            wr_data = i[7:0] * 8'h11;
            wr_en = 1'b1;
            @(posedge clk);
        end
        wr_en = 1'b0;
        @(posedge clk);
        
        $display("  Written 10 pixels sequentially");
        $display("  Reading back:\n");
        
        for (int i = 0; i < 10; i++) begin
            rd_addr = i[17:0];
            @(posedge clk);
            @(posedge clk); // Segundo ciclo para que rd_data se estabilice
            $display("    addr[0x%h] = 0x%h (expected: 0x%h)", i, rd_data, i[7:0] * 8'h11);
            if (rd_data == i[7:0] * 8'h11) $display("    PASS");
            else $display("    FAIL");
        end
        
        // --------------------------
        // TEST 3: Concurrent read/write
        // --------------------------
        $display("\nTest 3: Concurrent read/write");
        
        // Primero escribimos un valor en 0x100 para poder leerlo después
        wr_addr = 18'h100;
        wr_data = 8'h5A;
        wr_en = 1'b1;
        @(posedge clk);
        wr_en = 1'b0;
        @(posedge clk);
        
        // Ahora hacemos lectura y escritura simultáneas
        rd_addr = 18'h100;
        wr_addr = 18'h200;
        wr_data = 8'hCD;
        wr_en = 1'b1;
        @(posedge clk);
        wr_en = 1'b0;
        @(posedge clk); // Espera para que rd_data se estabilice
        
        $display("  Simultaneous read from 0x100 and write to 0x200");
        $display("  Read result from 0x100: 0x%h (expected: 0x5A)", rd_data);
        $display("  Write data to 0x200: 0xCD");
        
        if (rd_data == 8'h5A) $display("  READ PASS");
        else $display("  READ FAIL");
        
        // Verificar que la escritura en 0x200 fue exitosa
        rd_addr = 18'h200;
        @(posedge clk);
        @(posedge clk);
        $display("  Verification read from 0x200: 0x%h (expected: 0xCD)", rd_data);
        if (rd_data == 8'hCD) $display("  WRITE PASS\n");
        else $display("  WRITE FAIL\n");
        
        #20;
        $display("=== All Image Memory Tests Complete ===\n");
        $finish;
    end
endmodule