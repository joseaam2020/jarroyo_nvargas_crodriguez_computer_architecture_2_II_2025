module downscaler_fsm (
    input  logic clk,
    input  logic rst_n,

    // control
    input  logic start,
    input  logic step_mode,
    input  logic step_next,

    // address generator
    input  logic addr_gen_done,
    output logic addr_gen_start,
    output logic addr_gen_next,

    // interpolation
    output logic interpolate_start,
    input  logic interpolate_done,

    // writeback
    output logic result_write_en,

    // status
    output logic busy,
    output logic ready,
    output logic error
);

    // STATES
    localparam IDLE         = 3'd0;
    localparam FETCH        = 3'd1;
    localparam INTERPOLATE  = 3'd2;
    localparam WRITE        = 3'd3;
    localparam STEP_WAIT    = 3'd4;
    localparam DONE         = 3'd5;

    logic [2:0] state, next_state;

    // STATE REGISTER
    always_ff @(posedge clk or negedge rst_n) begin
        if (!rst_n)
            state <= IDLE;
        else
            state <= next_state;
    end

    // OUTPUTS + NEXT STATE LOGIC
    always_comb begin
        // defaults
        addr_gen_start    = 1'b0;
        addr_gen_next     = 1'b0;
        interpolate_start = 1'b0;
        result_write_en   = 1'b0;

        busy  = 1'b0;
        ready = 1'b0;
        error = 1'b0;

        next_state = state;

        case (state)

        // ---------------------------------------------------------
        // IDLE
        // ---------------------------------------------------------
        IDLE: begin
            ready = 1'b1;

            if (start) begin
                addr_gen_start = 1'b1;
                next_state = FETCH;
            end
        end

        // ---------------------------------------------------------
        // FETCH → pasa directo a INTERPOLATE
        // ---------------------------------------------------------
        FETCH: begin
            busy = 1'b1;

            // un pulso adicional por seguridad
            addr_gen_start = 1'b1;

            next_state = INTERPOLATE;
        end

        // ---------------------------------------------------------
        // INTERPOLATE
        // ---------------------------------------------------------
        INTERPOLATE: begin
            busy = 1'b1;
            interpolate_start = 1'b1;

            if (interpolate_done) begin
                result_write_en = 1'b1;

                if (!step_mode) begin
                    // ------------------ MODO AUTOMÁTICO -------------------

                    if (addr_gen_done) begin
                        next_state = DONE;
                    end else begin
                        addr_gen_next = 1'b1; // testbench lo espera aquí
                        next_state = FETCH;
                    end

                end else begin
                    // ------------------ MODO STEP --------------------------
                    next_state = WRITE;
                end
            end
        end

        // ---------------------------------------------------------
        // WRITE (solo en step mode)
        // ---------------------------------------------------------
        WRITE: begin
            busy = 1'b1;
            result_write_en = 1'b1;
            next_state = STEP_WAIT;
        end

        // ---------------------------------------------------------
        // STEP_WAIT – espera step_next
        // ---------------------------------------------------------
        STEP_WAIT: begin
            busy = 1'b1;

            if (step_mode) begin
                if (step_next) begin
                    if (addr_gen_done)
                        next_state = DONE;
                    else begin
                        addr_gen_next = 1'b1;
                        next_state = FETCH;
                    end
                end
            end else begin
                // fallback automático
                if (addr_gen_done)
                    next_state = DONE;
                else begin
                    addr_gen_next = 1'b1;
                    next_state = FETCH;
                end
            end
        end

        // ---------------------------------------------------------
        // DONE → IDLE
        // ---------------------------------------------------------
        DONE: begin
            ready = 1'b1;
            next_state = IDLE;
        end

        default: begin
            error = 1'b1;
            next_state = IDLE;
        end

        endcase
    end

endmodule