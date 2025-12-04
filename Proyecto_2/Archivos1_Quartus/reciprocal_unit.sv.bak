module reciprocal_unit #(
    parameter W = 16
)(
    input  logic clk,
    input  logic rst_n,
    input  logic start,
    input  logic [W-1:0] value,
    output logic [31:0] reciprocal,
    output logic valid
);

    logic [31:0] num;
    logic [W-1:0] denom;
    logic [31:0] result;
    logic [1:0] state;

    localparam IDLE = 0, RUN = 1, DONE = 2;

    always_ff @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            state <= IDLE;
            valid <= 0;
            result <= 0;
        end else begin

            case (state)

            IDLE: begin
                valid <= 0;
                if (start) begin
                    num   <= 32'h0001_0000;   // 1.0 en Q16
                    denom <= value;
                    result<= 0;
                    state <= RUN;
                end
            end

            RUN: begin
                if (num >= denom) begin
                    num    <= num - denom;
                    result <= result + 1;
                end else begin
                    state <= DONE;
                end
            end

            DONE: begin
                reciprocal <= result;
                valid      <= 1;
                state <= IDLE;
            end

            endcase
        end
    end

endmodule
