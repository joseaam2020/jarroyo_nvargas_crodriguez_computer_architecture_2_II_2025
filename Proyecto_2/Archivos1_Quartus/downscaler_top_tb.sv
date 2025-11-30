import fixed_point_pkg::*;

module downscaler_top_tb;

    // ============================================================
    // Señales
    // ============================================================
    logic clk, rst_n;
    logic [7:0]  reg_addr;
    logic        reg_wr_en, reg_rd_en;
    logic [31:0] reg_wr_data, reg_rd_data;
    logic [17:0] img_wr_addr;
    logic        img_wr_en;
    logic [7:0]  img_wr_data;
    logic [17:0] result_rd_addr;
    logic [7:0]  result_rd_data;

    // ============================================================
    // VARIABLES **GLOBALES** (REQUIRED BY QUARTUS)
    // ============================================================
    int i;
    int addr;
    int scale_int;
    int scale_frac_percent;
    int write_count;
    logic [31:0] status_tmp;

    // ============================================================
    // DUT
    // ============================================================
    downscaler_top dut (
        .clk(clk),
        .rst_n(rst_n),
        .reg_addr(reg_addr),
        .reg_wr_en(reg_wr_en),
        .reg_rd_en(reg_rd_en),
        .reg_wr_data(reg_wr_data),
        .reg_rd_data(reg_rd_data),
        .img_wr_addr(img_wr_addr),
        .img_wr_en(img_wr_en),
        .img_wr_data(img_wr_data),
        .result_rd_addr(result_rd_addr),
        .result_rd_data(result_rd_data)
    );

    // ============================================================
    // Clock
    // ============================================================
    initial begin
        clk = 0;
        forever #5 clk = ~clk;
    end

    // ============================================================
    // Task: Write register
    // ============================================================
    task write_reg(input [7:0] addr_in, input [31:0] data_in);
        @(posedge clk);
        reg_addr    = addr_in;
        reg_wr_data = data_in;
        reg_wr_en   = 1;
        @(posedge clk);
        reg_wr_en   = 0;
        @(posedge clk);
    endtask

    // ============================================================
    // Task: Read register
    // ============================================================
    task read_reg(input [7:0] addr_in, output [31:0] data_out);
        @(posedge clk);
        reg_addr  = addr_in;
        reg_rd_en = 1;
        @(posedge clk);
        @(posedge clk); 
        data_out  = reg_rd_data;
        reg_rd_en = 0;
        @(posedge clk);
    endtask

    // ============================================================
    // Task: Load image (256×256)
    // Quartus REQUIERE que "addr" esté declarado fuera del task.
    // ============================================================
    task load_image(input int width, input int height);
        $display("Cargando imagen...");
        for (addr = 0; addr < width * height; addr++) begin
            @(posedge clk);
            img_wr_addr = addr[17:0];
            img_wr_data = addr[7:0];
            img_wr_en   = 1;
            if (addr < 10)
                $display("  [CARGA] %0d -> addr=0x%05h, data=0x%02h",
                        addr, img_wr_addr, img_wr_data);
        end
        @(posedge clk);
        img_wr_en = 0;
        @(posedge clk);
        $display("Imagen cargada (%0d pixeles)", width * height);
    endtask

    // ============================================================
    // Task: Wait for ready
    // ============================================================
    task wait_for_ready();
        $display("=== Esperando procesamiento ===");
        for (i = 0; i < 100000; i++) begin
            @(posedge clk);
            read_reg(8'h05, status_tmp);
            if (i % 1000 == 0) begin
                read_reg(8'h06, reg_rd_data);
                $display("Ciclo %0d: Status=0x%h (ready=%b, busy=%b), Progress=%0d",
                         i, status_tmp, status_tmp[0], status_tmp[1], reg_rd_data);
            end
            if (status_tmp[0] == 1 && status_tmp[1] == 0) begin
                $display("¡Procesamiento completado en ciclo %0d!", i);
                return;
            end
        end
        $display("ERROR: timeout esperando ready");
        $finish;
    endtask

    // ============================================================
    // Monitor de escrituras en memoria destino
    // ============================================================
    always @(posedge clk) begin
        if (dut.dst_mem_wr_en) begin
            if (write_count < 10)
                $display("  [RESULTADO] %0d: addr=0x%05h, data=0x%02h",
                        write_count, dut.dst_mem_wr_addr, dut.dst_mem_wr_data);
            write_count++;
        end
    end

    // ============================================================
    // Test principal
    // ============================================================
    initial begin
        write_count     = 0;
        reg_wr_en       = 0;
        reg_rd_en       = 0;
        img_wr_en       = 0;
        reg_addr        = 0;
        img_wr_addr     = 0;
        result_rd_addr  = 0;
        reg_wr_data     = 0;

        rst_n = 0;
        repeat(5) @(posedge clk);
        rst_n = 1;
        repeat(5) @(posedge clk);

        $display("=== CONFIG INICIAL ===");
        write_reg(8'h00, 32'd256);
        write_reg(8'h01, 32'd256);
        write_reg(8'h02, 32'h0080);
        write_reg(8'h03, 32'h00000000);

        read_reg(8'h02, reg_rd_data);
        scale_int          = reg_rd_data[15:8];
        scale_frac_percent = (reg_rd_data[7:0] * 100) / 256;
        $display("Scale = %0d.%02d", scale_int, scale_frac_percent);

        // Cargar imagen
        load_image(256, 256);

        // Mostrar primeros píxeles
        for (i = 0; i < 10; i++)
            $display("  src[%0d] = 0x%02h",
                     i, dut.src_memory.mem[i]);

        // Iniciar procesamiento
        write_reg(8'h04, 32'h1);

        wait_for_ready();

        $display("\n=== LECTURA DESTINO ===");
        repeat(20) begin
            @(posedge clk);
            result_rd_addr = i;
            @(posedge clk);
            @(posedge clk);
            $display("dst[%0d] = 0x%02h", i, result_rd_data);
        end

        $finish;
    end

endmodule
