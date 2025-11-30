import fixed_point_pkg::*;

module control_registers #(
    parameter ADDR_WIDTH = 8
)(
    input  logic        clk,
    input  logic        rst_n,

    input  logic [ADDR_WIDTH-1:0] addr,
    input  logic        wr_en,
    input  logic        rd_en,
    input  logic [31:0] wr_data,
    output logic [31:0] rd_data,

    output logic [9:0]  img_width,
    output logic [9:0]  img_height,
    output fixed_point_t scale_factor,
    output logic        mode_simd,
    output logic [2:0]  simd_width,
    output logic        start,
    output logic        step_mode,
    output logic        step_next,

    input  logic        busy,
    input  logic        ready,
    input  logic        error,
    input  logic [31:0] progress,
    input  logic [31:0] flops_count,
    input  logic [31:0] mem_reads,
    input  logic [31:0] mem_writes
);

    localparam ADDR_IMG_WIDTH    = 8'h00;
    localparam ADDR_IMG_HEIGHT   = 8'h01;
    localparam ADDR_SCALE_FACTOR = 8'h02;
    localparam ADDR_MODE         = 8'h03;
    localparam ADDR_CONTROL      = 8'h04;
    localparam ADDR_STATUS       = 8'h05;
    localparam ADDR_PROGRESS     = 8'h06;
    localparam ADDR_FLOPS        = 8'h07;
    localparam ADDR_MEM_READS    = 8'h08;
    localparam ADDR_MEM_WRITES   = 8'h09;

    logic [9:0]  img_width_reg;
    logic [9:0]  img_height_reg;
    fixed_point_t scale_factor_reg;
    logic        mode_simd_reg;
    logic [2:0]  simd_width_reg;
    logic        start_reg;
    logic        step_mode_reg;
    logic        step_next_reg;

    always_ff @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            img_width_reg    <= 10'd512;
            img_height_reg   <= 10'd512;
            scale_factor_reg <= 16'h0100;
            mode_simd_reg    <= 1'b0;
            simd_width_reg   <= 3'd4;
            start_reg        <= 1'b0;
            step_mode_reg    <= 1'b0;
            step_next_reg    <= 1'b0;
        end else begin
            // Los pulsos se auto-limpian
            start_reg <= 1'b0;
            step_next_reg <= 1'b0;

            if (wr_en) begin
                case (addr)
                    ADDR_IMG_WIDTH:    img_width_reg    <= wr_data[9:0];
                    ADDR_IMG_HEIGHT:   img_height_reg   <= wr_data[9:0];
                    ADDR_SCALE_FACTOR: scale_factor_reg <= wr_data[15:0];
                    ADDR_MODE: begin
                        mode_simd_reg  <= wr_data[0];
                        simd_width_reg <= wr_data[3:1];
                    end
                    ADDR_CONTROL: begin
                        start_reg     <= wr_data[0];
                        step_mode_reg <= wr_data[1];
                        step_next_reg <= wr_data[2];
                    end
                    default: begin
                        // No hacer nada para direcciones no reconocidas
                    end
                endcase
            end
        end
    end

    always_comb begin
        rd_data = 32'h0;  // Valor por defecto
        
        if (rd_en) begin
            case (addr)
                ADDR_IMG_WIDTH:    rd_data = {22'h0, img_width_reg};
                ADDR_IMG_HEIGHT:   rd_data = {22'h0, img_height_reg};
                ADDR_SCALE_FACTOR: rd_data = {16'h0, scale_factor_reg};
                ADDR_MODE:         rd_data = {28'h0, simd_width_reg, mode_simd_reg};
                ADDR_CONTROL:      rd_data = {29'h0, step_mode_reg, 1'b0, start_reg};
                ADDR_STATUS:       rd_data = {29'h0, error, busy, ready};
                ADDR_PROGRESS:     rd_data = progress;
                ADDR_FLOPS:        rd_data = flops_count;
                ADDR_MEM_READS:    rd_data = mem_reads;
                ADDR_MEM_WRITES:   rd_data = mem_writes;
                default:           rd_data = 32'h0;  // Direcciones no reconocidas retornan 0
            endcase
        end
    end

    assign img_width    = img_width_reg;
    assign img_height   = img_height_reg;
    assign scale_factor = scale_factor_reg;
    assign mode_simd    = mode_simd_reg;
    assign simd_width   = simd_width_reg;
    assign start        = start_reg;
    assign step_mode    = step_mode_reg;
    assign step_next    = step_next_reg;

endmodule