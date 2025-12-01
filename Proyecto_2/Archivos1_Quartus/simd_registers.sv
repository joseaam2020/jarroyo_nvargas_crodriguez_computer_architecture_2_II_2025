module simd_registers #(
  parameter SIMD_WIDTH = 4,
  parameter NUM_REGS = 8,
  parameter DATA_WIDTH = 8
)(
  input logic clk,
  input logic rst_n,
  input logic [$clog2(NUM_REGS)-1:0] wr_reg_sel,
  input logic wr_en,
  input logic [SIMD_WIDTH*DATA_WIDTH-1:0] wr_data,
  input logic [$clog2(NUM_REGS)-1:0] rd_reg_sel_a,
  input logic [$clog2(NUM_REGS)-1:0] rd_reg_sel_b,
  output logic [SIMD_WIDTH*DATA_WIDTH-1:0] rd_data_a,
  output logic [SIMD_WIDTH*DATA_WIDTH-1:0] rd_data_b
);
  logic [SIMD_WIDTH*DATA_WIDTH-1:0] regs [0:NUM_REGS-1];
  
  always_ff @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
      for (int i = 0; i < NUM_REGS; i++) begin
        regs[i] <= '0;
      end
    end else if (wr_en && wr_reg_sel < NUM_REGS) begin
      regs[wr_reg_sel] <= wr_data;
    end
  end
  
  assign rd_data_a = (rd_reg_sel_a < NUM_REGS) ? regs[rd_reg_sel_a] : '0;
  assign rd_data_b = (rd_reg_sel_b < NUM_REGS) ? regs[rd_reg_sel_b] : '0;
endmodule