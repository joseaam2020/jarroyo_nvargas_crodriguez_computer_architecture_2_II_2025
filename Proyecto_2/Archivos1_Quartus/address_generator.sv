import fixed_point_pkg::*;

module address_generator (
    input  logic         clk,
    input  logic         rst_n,
    input  logic         start,
    input  logic         next_pixel,
    
    // Dimensiones de imagen original (enteros)
    input  logic [9:0]   src_width,
    input  logic [9:0]   src_height,
    
    // Factor de escala en punto fijo (8.8)
    input  fixed_point_t scale_factor,
    
    // Direcciones de los 4 píxeles vecinos
    output logic [17:0]  addr_tl,
    output logic [17:0]  addr_tr,
    output logic [17:0]  addr_bl,
    output logic [17:0]  addr_br,
    
    // Pesos de interpolación en punto fijo
    output fixed_point_t weight_x,
    output fixed_point_t weight_y,
    
    // Control
    output logic         valid,
    output logic         done
);

    // ========================================
    // ESTADOS
    // ========================================
    typedef enum logic [1:0] {
        IDLE,
        PROCESSING,
        DONE_ST
    } state_t;
    
    state_t state;
    
    // ========================================
    // REGISTROS INTERNOS
    // ========================================
    
    // Dimensiones destino (enteros)
    logic [9:0] dst_width, dst_height;
    
    // Coordenadas destino actuales (enteros)
    logic [9:0] dst_x, dst_y;
    
    // Coordenadas fuente en punto fijo (16 bits = 8.8)
    fixed_point_t src_x_fixed, src_y_fixed;
    
    // Partes entera y fraccionaria
    logic [9:0] src_x_int, src_y_int;
    logic [7:0] src_x_frac, src_y_frac;
    
    // Variables temporales para cálculos
    logic [25:0] temp_x, temp_y;
    logic [19:0] temp_width, temp_height;

    // ========================================
    // MÁQUINA DE ESTADOS
    // ========================================
    always_ff @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            state      <= IDLE;
            dst_x      <= 10'h0;
            dst_y      <= 10'h0;
            dst_width  <= 10'h0;
            dst_height <= 10'h0;
            valid      <= 1'b0;
            done       <= 1'b0;
        end else begin
            case (state)
                // ----------------------------------------
                // IDLE - Esperar inicio
                // ----------------------------------------
                IDLE: begin
                    valid <= 1'b0;
                    done  <= 1'b0;
                    
                    if (start) begin
                        // Calcular dimensiones destino usando punto fijo
                        // dst_dim = (src_dim * scale_factor) >> 8
                        temp_width  = src_width * scale_factor;
                        temp_height = src_height * scale_factor;
                        
                        dst_width  <= temp_width[17:8];   // Dividir por 256
                        dst_height <= temp_height[17:8];
                        
                        dst_x <= 10'h0;
                        dst_y <= 10'h0;
                        
                        state <= PROCESSING;
                        valid <= 1'b1;
                    end
                end
                
                // ----------------------------------------
                // PROCESSING - Generar direcciones
                // ----------------------------------------
                PROCESSING: begin
                    valid <= 1'b1;
                    
                    if (next_pixel) begin
                        // Verificar si terminamos ANTES de avanzar
                        if (dst_x == dst_width - 1 && dst_y == dst_height - 1) begin
                            // Último píxel procesado
                            state <= DONE_ST;
                            valid <= 1'b0;
                            done  <= 1'b1;
                        end else if (dst_x == dst_width - 1) begin
                            // Fin de fila, ir a siguiente
                            dst_x <= 10'h0;
                            dst_y <= dst_y + 1;
                        end else begin
                            // Siguiente columna
                            dst_x <= dst_x + 1;
                        end
                    end
                end
                
                // ----------------------------------------
                // DONE - Finalizado
                // ----------------------------------------
                DONE_ST: begin
                    done  <= 1'b1;
                    valid <= 1'b0;
                    
                    // Volver a IDLE si se reinicia
                    if (start) begin
                        state <= IDLE;
                    end
                end
                
                default: begin
                    state <= IDLE;
                end
            endcase
        end
    end

    // ========================================
    // LÓGICA COMBINACIONAL - CÁLCULO DE COORDENADAS Y DIRECCIONES
    // ========================================
    always_comb begin
        // ----------------------------------------
        // Calcular coordenadas fuente en punto fijo
        // ----------------------------------------
        // Fórmula: src = dst / scale_factor
        // En punto fijo 8.8: src_fixed = (dst * 256) / scale_factor
        // Multiplicamos dst por 256 (shift 8) y dividimos por scale
        
        // Evitar división por cero
        if (scale_factor != 16'h0000) begin
            temp_x = ({dst_x, 16'h0000}) / scale_factor;
            temp_y = ({dst_y, 16'h0000}) / scale_factor;
        end else begin
            temp_x = 26'h0;
            temp_y = 26'h0;
        end
        
        src_x_fixed = temp_x[15:0];
        src_y_fixed = temp_y[15:0];
        
        // ----------------------------------------
        // Extraer parte entera y fraccionaria
        // ----------------------------------------
        // De fixed_point_t [15:0] en formato 8.8:
        src_x_int  = src_x_fixed[15:8];  // 8 bits superiores = parte entera
        src_y_int  = src_y_fixed[15:8];
        
        src_x_frac = src_x_fixed[7:0];   // 8 bits inferiores = parte fraccionaria
        src_y_frac = src_y_fixed[7:0];
        
        // ----------------------------------------
        // Pesos de interpolación (formato 0.8)
        // ----------------------------------------
        weight_x = {8'h00, src_x_frac};
        weight_y = {8'h00, src_y_frac};
        
        // ----------------------------------------
        // Calcular direcciones lineales con bounds checking
        // ----------------------------------------
        
        // Top-Left (TL) - siempre válido
        addr_tl = src_y_int * src_width + src_x_int;
        
        // Top-Right (TR) - verificar límite horizontal
        if (src_x_int < src_width - 1)
            addr_tr = src_y_int * src_width + (src_x_int + 10'h1);
        else
            addr_tr = addr_tl;  // Clamping al borde
        
        // Bottom-Left (BL) - verificar límite vertical
        if (src_y_int < src_height - 1)
            addr_bl = (src_y_int + 10'h1) * src_width + src_x_int;
        else
            addr_bl = addr_tl;  // Clamping al borde
        
        // Bottom-Right (BR) - verificar ambos límites
        if ((src_x_int < src_width - 1) && (src_y_int < src_height - 1))
            addr_br = (src_y_int + 10'h1) * src_width + (src_x_int + 10'h1);
        else
            addr_br = addr_tl;  // Clamping al borde
    end

endmodule