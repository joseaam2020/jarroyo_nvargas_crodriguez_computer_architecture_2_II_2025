// address_generator_simd.sv
// Generador de direcciones SIMD: N píxeles por ciclo (N >= 4).
// Requiere fixed_point_pkg (same as original): fixed_point_t assumed 16-bit with 8 fractional bits.

`timescale 1ns/1ps
import fixed_point_pkg::*;

module address_generator_simd #(
  parameter int SIMD_WIDTH = 8,            // N píxeles por ciclo. Debe cumplirse: SIMD_WIDTH >= 4
  parameter int ADDR_WIDTH = 18,           // ancho de direcciones de memoria (ej. para imágenes)
  parameter int COORD_WIDTH = 10           // ancho de coordenadas (dst/src)
)(
  input  logic                         clk,
  input  logic                         rst_n,
  input  logic                         start,
  input  logic                         next_pixel,      // avanzar al siguiente grupo SIMD
  input  logic [COORD_WIDTH-1:0]       src_width,
  input  logic [COORD_WIDTH-1:0]       src_height,
  input  fixed_point_t                 scale_factor,    // formato fijo compatible con fixed_point_pkg
  // Salidas por lane (arrays)
  output logic [ADDR_WIDTH-1:0]        addr_tl  [SIMD_WIDTH-1:0],
  output logic [ADDR_WIDTH-1:0]        addr_tr  [SIMD_WIDTH-1:0],
  output logic [ADDR_WIDTH-1:0]        addr_bl  [SIMD_WIDTH-1:0],
  output logic [ADDR_WIDTH-1:0]        addr_br  [SIMD_WIDTH-1:0],
  output fixed_point_t                 weight_x [SIMD_WIDTH-1:0],
  output fixed_point_t                 weight_y [SIMD_WIDTH-1:0],
  output logic [SIMD_WIDTH-1:0]        lane_valid,      // 1 si la lane produce un pixel válido
  output logic                         valid,           // todo el vector válido
  output logic                         done
);

  // -----------------------------------------------------------------------
  // Static checks
  // -----------------------------------------------------------------------
  initial begin
    if (SIMD_WIDTH < 4) begin
      $error("SIMD_WIDTH must be >= 4");
    end
  end

  // -----------------------------------------------------------------------
  // FSM: IDLE -> PROCESSING -> DONE
  // -----------------------------------------------------------------------
  typedef enum logic [1:0] {IDLE, PROCESSING, DONE_ST} state_t;
  state_t state, next_state;

  // Contadores de destino
  logic [COORD_WIDTH-1:0] dst_width, dst_height;
  logic [COORD_WIDTH-1:0] dst_x_base;   // posicion del primer pixel del grupo (por ciclo)
  logic [COORD_WIDTH-1:0] dst_y;

  // Señales auxiliares
  logic [COORD_WIDTH-1:0] next_dst_x_base;
  logic [COORD_WIDTH-1:0] next_dst_y;
  logic at_end;
  
  // Señales temporales para cálculo de dimensiones
  logic [2*COORD_WIDTH+8-1:0] tmp_w, tmp_h;

  // valid cuando estamos procesando
  always_comb begin
    valid = (state == PROCESSING);
  end

  // FSM state register
  always_ff @(posedge clk or negedge rst_n) begin
    if (!rst_n) state <= IDLE;
    else state <= next_state;
  end

  // next-state logic
  always_comb begin
    next_state = state;
    case (state)
      IDLE: if (start) next_state = PROCESSING;
      PROCESSING: if (next_pixel && at_end) next_state = DONE_ST;
      DONE_ST: if (start) next_state = IDLE;
      default: next_state = IDLE;
    endcase
  end

  // Inicialización / counters
  always_ff @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
      dst_x_base <= '0;
      dst_y <= '0;
      dst_width <= '0;
      dst_height <= '0;
      done <= 1'b0;
    end else begin
      case (state)
        IDLE: begin
          done <= 1'b0;
          if (start) begin
            // calcular dimensiones destino: dst = src * scale_factor
            // scale_factor es fixed_point_t (8.8): multiplicación devuelve width + frac
            // tomamos la parte entera (desplazamiento 8 bits)
            dst_width <= COORD_WIDTH'((src_width * scale_factor) >> 8);
            dst_height <= COORD_WIDTH'((src_height * scale_factor) >> 8);
            dst_x_base <= '0;
            dst_y <= '0;
          end
        end

        PROCESSING: begin
          if (next_pixel) begin
            // avance por grupo SIMD (cada ciclo produce SIMD_WIDTH píxeles)
            // si el avance sale de fila, incrementa y reinicia x
            if (dst_x_base + COORD_WIDTH'(SIMD_WIDTH) >= dst_width) begin
              next_dst_x_base = '0;
              next_dst_y = dst_y + COORD_WIDTH'(1);
            end else begin
              next_dst_x_base = dst_x_base + COORD_WIDTH'(SIMD_WIDTH);
              next_dst_y = dst_y;
            end

            dst_x_base <= next_dst_x_base;
            dst_y <= next_dst_y;
            // marcar fin si el siguiente next_dst_y >= dst_height
            // (at_end calculado en combinacional)
          end
        end

        DONE_ST: begin
          done <= 1'b1;
          // mantener contadores
        end

        default: ;
      endcase
    end
  end

  // ---------------------------------------------------------
  // Lógica combinacional: calcular direcciones y pesos por lane
  // ---------------------------------------------------------
  // Reutilización inteligente:
  // src_pos_fixed_for_leader = (dst_pos_leader << 8) / scale_factor
  // delta_fixed = (1 << 8) / scale_factor  -> al sumar delta_fixed obtenemos el siguiente pixel en x en punto fijo
  //
  // Para Y: src_y_fixed es común a todas las lanes (mismo dst_y)
  //

  // Variables temporales
  logic [25:0] temp_x_leader;        // guarda (dst_x_base << 8) / scale_factor (suficiente ancho)
  logic [25:0] temp_y_fixed;         // (dst_y << 8) / scale_factor
  fixed_point_t src_x_fixed_leader;  // 16 bits (8.8)
  fixed_point_t src_y_fixed;
  fixed_point_t delta_fixed;         // incremento fijo por +1 en dst_x: 256 / scale_factor
  
  // Variables para el bucle de lanes (declaradas fuera)
  fixed_point_t src_x_lane_fixed [SIMD_WIDTH];
  logic [COORD_WIDTH-1:0] src_x_int [SIMD_WIDTH];
  logic [COORD_WIDTH-1:0] src_y_int [SIMD_WIDTH];
  logic [7:0] src_x_frac [SIMD_WIDTH];
  logic [7:0] src_y_frac [SIMD_WIDTH];
  logic [ADDR_WIDTH-1:0] base_addr_tl [SIMD_WIDTH];
  logic [COORD_WIDTH-1:0] dst_x_lane [SIMD_WIDTH];

  // Cálculo combinacional
  always_comb begin
    // valores por defecto
    lane_valid = '0;
    // por defecto pesos 0
    for (int i = 0; i < SIMD_WIDTH; i++) begin
      weight_x[i] = '0;
      weight_y[i] = '0;
      addr_tl[i] = '0;
      addr_tr[i] = '0;
      addr_bl[i] = '0;
      addr_br[i] = '0;
    end

    // Si scale_factor = 0 (evitar división por 0)
    if (scale_factor == 16'h0000) begin
      // nada válido
      temp_x_leader = '0;
      temp_y_fixed = '0;
      src_x_fixed_leader = '0;
      src_y_fixed = '0;
      delta_fixed = '0;
    end else begin
      // calculamos src_x_fixed para el pixel líder (dst_x_base)
      // src = (dst << 8) / scale_factor
      // Implementación: usamos enteros anchos para división
      temp_x_leader = 26'(({16'h0, dst_x_base, 8'h00}) / {10'h0, scale_factor}); // (dst_x_base << 8) / scale
      temp_y_fixed  = 26'(({16'h0, dst_y, 8'h00}) / {10'h0, scale_factor});

      // casteos
      src_x_fixed_leader = temp_x_leader[15:0];
      src_y_fixed = temp_y_fixed[15:0];

      // delta: incremento en src_fixed cuando dst_x aumenta en 1
      // delta_fixed = (1 << 8) / scale_factor  (representado en mismo formato)
      delta_fixed = (16'd256) / scale_factor;

      // Para cada lane, calcular src_x_fixed_lane = src_x_fixed_leader + i*delta_fixed
      for (int lane = 0; lane < SIMD_WIDTH; lane++) begin
        dst_x_lane[lane] = dst_x_base + lane[COORD_WIDTH-1:0];

        // Compruebo si dst_x_lane está dentro de dst_width y dst_y dentro de dst_height
        if (dst_x_lane[lane] < dst_width && dst_y < dst_height) begin
          lane_valid[lane] = 1'b1;
        end else begin
          lane_valid[lane] = 1'b0;
        end

        // Calculo src_x_lane_fixed reutilizando delta
        // src_x = src_x_fixed_leader + lane * delta_fixed
        // Multiplicación lane*delta_fixed es pequeña (lane <= SIMD_WIDTH-1)
        src_x_lane_fixed[lane] = src_x_fixed_leader + (delta_fixed * 16'(lane));

        // src_y es común a todas las lanes
        src_x_int[lane] = src_x_lane_fixed[lane][15:8];
        src_y_int[lane] = src_y_fixed[15:8];
        src_x_frac[lane] = src_x_lane_fixed[lane][7:0];
        src_y_frac[lane] = src_y_fixed[7:0];

        // pesos (extender a fixed_point_t si tu formato es wider)
        weight_x[lane] = {8'h00, src_x_frac[lane]};
        weight_y[lane] = {8'h00, src_y_frac[lane]};

        // Direcciones con clamping
        // addr = src_y_int * src_width + src_x_int
        base_addr_tl[lane] = ADDR_WIDTH'(src_y_int[lane] * src_width + src_x_int[lane]);
        addr_tl[lane] = base_addr_tl[lane];

        if (src_x_int[lane] < src_width - COORD_WIDTH'(1))
          addr_tr[lane] = ADDR_WIDTH'(base_addr_tl[lane] + 1);
        else
          addr_tr[lane] = base_addr_tl[lane];

        if (src_y_int[lane] < src_height - COORD_WIDTH'(1))
          addr_bl[lane] = ADDR_WIDTH'((src_y_int[lane] + COORD_WIDTH'(1)) * src_width + src_x_int[lane]);
        else
          addr_bl[lane] = base_addr_tl[lane];

        if (src_x_int[lane] < src_width - COORD_WIDTH'(1) && src_y_int[lane] < src_height - COORD_WIDTH'(1))
          addr_br[lane] = ADDR_WIDTH'((src_y_int[lane] + COORD_WIDTH'(1)) * src_width + src_x_int[lane] + COORD_WIDTH'(1));
        else
          addr_br[lane] = base_addr_tl[lane];
      end
    end

    // at_end: si al avanzar un grupo SIMD quedamos fuera de altura
    // Si dst_y == dst_height-1 y dst_x_base + SIMD_WIDTH >= dst_width entonces el siguiente avance sale.
    at_end = ( (dst_y == dst_height - COORD_WIDTH'(1)) && (dst_x_base + COORD_WIDTH'(SIMD_WIDTH) >= dst_width) );
  end

  // ---------------------------------------------------------
  // BUFFER SIMD empaquetado (para carga/almacenamiento temporal y reutilización)
  // ---------------------------------------------------------
  // Note: estos registros no forman parte estricta del address generator,
  // pero se proveen como ejemplo de diseño de registros SIMD empaquetados:
  // - permiten carga simultánea de N bytes desde memoria (ej: un bus de datos ancho)
  // - almacenan filas de píxeles vecinos y permiten compartir píxeles entre salidas adyacentes.
  //
  // Implementación: dos filas empaquetadas (row0, row1) de (SIMD_WIDTH + 1) bytes cada una.
  // El +1 permite leer el vecino derecho necesario para la interpolación (tr/br).
  //
  localparam int ROW_PACKED_BYTES = SIMD_WIDTH + 1;
  logic [8*ROW_PACKED_BYTES-1:0] simd_row0_packed; // fila y: bytes[0]..bytes[N] empaquetados
  logic [8*ROW_PACKED_BYTES-1:0] simd_row1_packed;

  // Ejemplo de interfaz para cargar: se asume que el controlador de memoria puede escribir
  // simultáneamente un vector de ROW_PACKED_BYTES bytes en simd_row{0,1}_packed (por ciclo).
  // Aquí solo mostramos cómo rotar/actualizar las ventanas en cada advance (next_pixel).
  always_ff @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
      simd_row0_packed <= '0;
      simd_row1_packed <= '0;
    end else begin
      if (state == PROCESSING && next_pixel) begin
        // Ejemplo de desplazamiento de ventana: desplazamos a la izquierda para simular avance de grupo
        // En la práctica, la carga vendría desde la memoria usando las direcciones calculadas arriba.
        // Aquí lo dejamos en 'auto-rotate' para ilustrar la reutilización sin lecturas redundantes.
        simd_row0_packed <= {simd_row0_packed[8*(ROW_PACKED_BYTES-1)-1:0], 8'h00}; // SHIFT LEFT + nuevo byte en LSB (placeholder)
        simd_row1_packed <= {simd_row1_packed[8*(ROW_PACKED_BYTES-1)-1:0], 8'h00};
      end
    end
  end

  // ---------------------------------------------------------
  // Fin del módulo
  // ---------------------------------------------------------
endmodule
