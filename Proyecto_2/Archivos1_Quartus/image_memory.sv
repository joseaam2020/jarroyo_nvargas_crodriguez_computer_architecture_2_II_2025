module image_memory #(
    parameter ADDR_WIDTH = 18,
    parameter DATA_WIDTH = 8,
    parameter MEM_SIZE   = 262144
)(
    input  logic                     clk,

    input  logic [ADDR_WIDTH-1:0]    rd_addr,
    output logic [DATA_WIDTH-1:0]    rd_data,

    input  logic [ADDR_WIDTH-1:0]    wr_addr,
    input  logic                     wr_en,
    input  logic [DATA_WIDTH-1:0]    wr_data
);

    // Memoria sintetizable para Intel/Altera
    logic [DATA_WIDTH-1:0] mem [0:MEM_SIZE-1];

    // Escritura sincronizada
    always_ff @(posedge clk) begin
        if (wr_en) begin
            mem[wr_addr] <= wr_data;
        end
    end

    // Lectura sincronizada (1 ciclo de latencia)
    always_ff @(posedge clk) begin
        rd_data <= mem[rd_addr];
    end

endmodule