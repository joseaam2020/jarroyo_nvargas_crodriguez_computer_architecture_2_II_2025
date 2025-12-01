module performance_counters (
    input  logic        clk,
    input  logic        rst_n,

    input  logic        clear,

    input  logic        flop_inc,
    input  logic [31:0] flop_count,
    input  logic        mem_read_inc,
    input  logic        mem_write_inc,

    output logic [31:0] total_flops,
    output logic [31:0] total_mem_reads,
    output logic [31:0] total_mem_writes
);

    always_ff @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            total_flops <= 32'h0;
            total_mem_reads <= 32'h0;
            total_mem_writes <= 32'h0;
        end else begin
            if (clear) begin
                total_flops <= 32'h0;
                total_mem_reads <= 32'h0;
                total_mem_writes <= 32'h0;
            end else begin
                if (flop_inc) begin
                    total_flops <= total_flops + flop_count;
                end
                if (mem_read_inc) begin
                    total_mem_reads <= total_mem_reads + 1;
                end
                if (mem_write_inc) begin
                    total_mem_writes <= total_mem_writes + 1;
                end
            end
        end
    end

endmodule