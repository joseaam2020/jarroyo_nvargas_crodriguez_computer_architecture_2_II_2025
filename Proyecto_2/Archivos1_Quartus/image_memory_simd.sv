// image_memory_simd.sv
// Memoria byte-addressable que entrega SIMD_WIDTH bytes contiguos por lectura.
// Escritura SIMD: escribe SIMD_WIDTH bytes contiguos en una operación.
`timescale 1ns/1ps
module image_memory_simd #(
    parameter int ADDR_WIDTH = 18,           // bits de dirección (direcciones en bytes)
    parameter int DATA_WIDTH = 8,            // anchura por elemento (bytes)
    parameter int SIMD_WIDTH = 8,            // número de bytes devueltos por lectura
    parameter int MEM_SIZE   = 262144        // número de elementos (bytes) en la memoria
)(
    input  logic                         clk,

    // lectura SIMD: dirección base (primer byte), salida empaquetada (MSB: lane 0)
    input  logic [ADDR_WIDTH-1:0]        rd_base_addr,
    output logic [SIMD_WIDTH*DATA_WIDTH-1:0] rd_data_packed, // [SIMD_WIDTH-1:0] bytes packed as [MSB .. LSB]

    // escritura SIMD: dirección base, wr_en escribe SIMD_WIDTH bytes contiguos
    input  logic [ADDR_WIDTH-1:0]        wr_base_addr,
    input  logic                         wr_en,
    input  logic [SIMD_WIDTH*DATA_WIDTH-1:0] wr_data_packed
);

    // Memoria (byte-addressable)
    logic [DATA_WIDTH-1:0] mem [0:MEM_SIZE-1];

    // --- Lectura síncrona (1 ciclo de latencia) ---
    // Guardamos la dirección base y a la siguiente salida leemos mem[addr + i]
    logic [ADDR_WIDTH-1:0] rd_base_addr_q;

    always_ff @(posedge clk) begin
        rd_base_addr_q <= rd_base_addr;

        // Empaquetar la salida un ciclo después
        // Nota: hacemos chequeo de límites para evitar access out-of-range en síntesis segura
        for (int i = 0; i < SIMD_WIDTH; i++) begin
            logic [ADDR_WIDTH-1:0] addr_i;
            addr_i = ADDR_WIDTH'(rd_base_addr_q + i);
            if (addr_i < ADDR_WIDTH'(MEM_SIZE))
                rd_data_packed[ (SIMD_WIDTH-i)*DATA_WIDTH-1 -: DATA_WIDTH ] <= mem[ addr_i ];
            else
                rd_data_packed[ (SIMD_WIDTH-i)*DATA_WIDTH-1 -: DATA_WIDTH ] <= {DATA_WIDTH{1'b0}};
        end
    end

    // --- Escritura síncrona ---
    // Cuando wr_en = 1, escribimos SIMD_WIDTH bytes contiguos comenzando en wr_base_addr
    always_ff @(posedge clk) begin
        if (wr_en) begin
            for (int i = 0; i < SIMD_WIDTH; i++) begin
                logic [ADDR_WIDTH-1:0] addr_i;
                addr_i = ADDR_WIDTH'(wr_base_addr + i);
                if (addr_i < ADDR_WIDTH'(MEM_SIZE))
                    mem[ addr_i ] <= wr_data_packed[ (SIMD_WIDTH-i)*DATA_WIDTH-1 -: DATA_WIDTH ];
                // si addr_i fuera fuera de rango, se ignora la escritura
            end
        end
    end

endmodule
