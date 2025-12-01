import fixed_point_pkg::*;

module address_generator (
  input logic clk, rst_n, start, next_pixel,
  input logic [9:0] src_width, src_height,
  input fixed_point_t scale_factor,
  input logic [2:0] simd_width,
  output logic [17:0] addr_tl, addr_tr, addr_bl, addr_br,
  output fixed_point_t weight_x, weight_y,
  output logic valid, done
);
  typedef enum logic [1:0] {IDLE, PROCESSING, DONE_ST} state_t;
  state_t state, next_state;
  
  // Registros internos
  logic [9:0] dst_width, dst_height, dst_x, dst_y;
  logic [9:0] next_dst_x, next_dst_y;
  logic [9:0] src_x_int, src_y_int;
  logic [7:0] src_x_frac, src_y_frac;
  fixed_point_t src_x_fixed, src_y_fixed;
  logic [25:0] temp_x, temp_y;
  logic [19:0] temp_width, temp_height;
  logic [2:0] effective_simd_width;
  logic at_end;
  logic processing_last_pixel;
  
  // SIMD width efectivo (0 se interpreta como 1)
  assign effective_simd_width = (simd_width == 3'd0) ? 3'd1 : simd_width;
  
  // ⭐ LÓGICA CORREGIDA DE DETECCIÓN DE FIN
  always_comb begin
    // Calcular siguiente posición DESPUÉS del avance
    if (dst_x + effective_simd_width >= dst_width) begin
      next_dst_x = 10'h0;
      next_dst_y = dst_y + 10'h1;
    end else begin
      next_dst_x = dst_x + effective_simd_width;
      next_dst_y = dst_y;
    end
    
    // ⭐ CORRECCIÓN: Detectar si ESTAMOS procesando el último pixel
    // El último pixel es cuando:
    // - Estamos en la última fila (dst_y == dst_height - 1) Y
    // - El siguiente avance nos sacaría de la fila (dst_x + simd_width >= dst_width)
    processing_last_pixel = (dst_y == dst_height - 1) && 
                           (dst_x + effective_simd_width >= dst_width);
    
    // at_end se activa DESPUÉS de procesar el último pixel
    at_end = (next_dst_y >= dst_height);
  end
  
  // FSM State Register
  always_ff @(posedge clk or negedge rst_n) begin
    if (!rst_n)
      state <= IDLE;
    else
      state <= next_state;
  end
  
  // FSM Next State Logic
  always_comb begin
    next_state = state;
    
    case (state)
      IDLE: begin
        if (start)
          next_state = PROCESSING;
      end
      
      PROCESSING: begin
        // ⭐ CORRECCIÓN: Ir a DONE solo DESPUÉS de avanzar past el último pixel
        if (next_pixel && at_end)
          next_state = DONE_ST;
      end
      
      DONE_ST: begin
        if (start)
          next_state = IDLE;
      end
      
      default: next_state = IDLE;
    endcase
  end
  
  // FSM Output Logic
  always_comb begin
    valid = (state == PROCESSING);
    done = (state == DONE_ST);
  end
  
  // Counter Logic
  always_ff @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
      dst_x <= 10'h0; 
      dst_y <= 10'h0;
      dst_width <= 10'h0; 
      dst_height <= 10'h0;
    end else begin
      case (state)
        IDLE: begin
          if (start) begin
            // Calcular dimensiones de destino
            temp_width = src_width * scale_factor;
            temp_height = src_height * scale_factor;
            dst_width <= temp_width[17:8];
            dst_height <= temp_height[17:8];
            dst_x <= 10'h0; 
            dst_y <= 10'h0;
          end
        end
        
        PROCESSING: begin
          if (next_pixel) begin
            dst_x <= next_dst_x;
            dst_y <= next_dst_y;
          end
        end
        
        DONE_ST: begin
          // Mantener valores
        end
        
        default: begin
          dst_x <= 10'h0;
          dst_y <= 10'h0;
        end
      endcase
    end
  end
  
  // Cálculo de coordenadas fuente (combinacional)
  always_comb begin
    if (scale_factor != 16'h0000) begin
      // Para downscaling/upscaling: src_pos = dst_pos / scale_factor
      // src = (dst << 8) / scale_factor (para mantener precisión)
      temp_x = {dst_x, 16'h0} / {10'h0, scale_factor};  
      temp_y = {dst_y, 16'h0} / {10'h0, scale_factor};
    end else begin
      temp_x = 26'h0; 
      temp_y = 26'h0;
    end
    
    src_x_fixed = temp_x[15:0];
    src_y_fixed = temp_y[15:0];
    
    src_x_int = src_x_fixed[15:8];
    src_y_int = src_y_fixed[15:8];
    src_x_frac = src_x_fixed[7:0];
    src_y_frac = src_y_fixed[7:0];
    
    weight_x = {8'h00, src_x_frac};
    weight_y = {8'h00, src_y_frac};
    
    // Direcciones con clamping
    addr_tl = src_y_int * src_width + src_x_int;
    
    if (src_x_int < src_width - 1)
      addr_tr = src_y_int * src_width + src_x_int + 10'h1;
    else
      addr_tr = addr_tl;
    
    if (src_y_int < src_height - 1)
      addr_bl = (src_y_int + 10'h1) * src_width + src_x_int;
    else
      addr_bl = addr_tl;
    
    if (src_x_int < src_width - 1 && src_y_int < src_height - 1)
      addr_br = (src_y_int + 10'h1) * src_width + src_x_int + 10'h1;
    else
      addr_br = addr_tl;
  end
endmodule