#include "file_line_selector.h"
#include "interconnect.h"
#include "mem.h"
#include "processing_element.h"
#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Table.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <thread>

#define NUM_PE 4
#define NUM_REG 8
#define CACHE_SETS 8
#define CACHE_WAYS 2
#define DATA_PER_CACHE 4
#define BLOCKS 128
#define ADDRS_PER_BLOCK 4


int exe_index = 0;

// ==========================================
// Table para Memoria
// ==========================================
class MemoryTable : public Fl_Table {
  std::vector<std::vector<double>> memory_data;

public:
  MemoryTable(int X, int Y, int W, int H, const char *L = 0)
      : Fl_Table(X, Y, W, H, L) {
    rows(BLOCKS);
    cols(ADDRS_PER_BLOCK + 1);
    row_height_all(30);
    col_width_all(100);
    col_header(1);
    row_header(1);
    col_resize(1);
    row_resize(1);
    end();
  }

  void set_memory(const std::vector<std::vector<double>> &mem) {
    memory_data = mem;
    redraw();
  }

protected:
  void draw_cell(TableContext context, int R, int C, int X, int Y, int W,
                 int H) override {
    char s[40];
    switch (context) {
    case CONTEXT_CELL:
      fl_push_clip(X, Y, W, H);
      fl_color(FL_WHITE);
      fl_rectf(X, Y, W, H);
      fl_color(FL_BLACK);
      fl_rect(X, Y, W, H);
      if (C == 0)
        snprintf(s, sizeof(s), "Bloque %d", R);
      else if (R < static_cast<int>(memory_data.size()) &&
               C - 1 < static_cast<int>(memory_data[R].size()))
        snprintf(s, sizeof(s), "%.2f", memory_data[R][C - 1]);
      else
        snprintf(s, sizeof(s), "0.00");
      fl_draw(s, X + 4, Y, W - 4, H, FL_ALIGN_LEFT);
      fl_pop_clip();
      break;
    default:
      break;
    }
  }
};

// ==========================================
// Table para Registros
// ==========================================
class RegTable : public Fl_Table {
  ProcessingElement *pe;

public:
  RegTable(int X, int Y, int W, int H, ProcessingElement *pe)
      : Fl_Table(X, Y, W, H), pe(pe) {
    rows(NUM_REG);
    cols(2);
    row_header(0);
    col_header(1);
    col_resize(1);
    end();
  }

private:
  void draw_cell(TableContext context, int R, int C, int X, int Y, int W,
                 int H) override {
    char s[40];
    switch (context) {
    case CONTEXT_COL_HEADER:
      fl_push_clip(X, Y, W, H);
      fl_draw_box(FL_FLAT_BOX, X, Y, W, H, fl_rgb_color(200, 200, 200));
      fl_color(FL_BLACK);
      fl_draw(C == 0 ? "Registro" : "Valor", X, Y, W, H, FL_ALIGN_CENTER);
      fl_pop_clip();
      break;
    case CONTEXT_CELL:
      fl_push_clip(X, Y, W, H);
      fl_color(FL_WHITE);
      fl_rectf(X, Y, W, H);
      fl_color(FL_BLACK);
      if (C == 0)
        snprintf(s, sizeof(s), "R%d", R);
      else
        snprintf(s, sizeof(s), "%.4f", pe->getRegisters()[R]);
      fl_draw(s, X + 4, Y, W - 4, H, FL_ALIGN_LEFT);
      fl_rect(X, Y, W, H);
      fl_pop_clip();
      break;
    default:
      break;
    }
  }
};

// ==========================================
// Table para Cache
// ==========================================
class CacheTable : public Fl_Table {
  ProcessingElement *pe;

public:
  CacheTable(int X, int Y, int W, int H, ProcessingElement *pe)
      : Fl_Table(X, Y, W, H), pe(pe) {
    rows(CACHE_SETS * CACHE_WAYS);
    cols(4 +
         DATA_PER_CACHE); // Set, Way, Tag, Usage, Data0, Data1, Data2, Data3
    row_header(0);
    col_header(1);
    col_resize(1);
    end();
  }

private:
  void draw_cell(TableContext context, int R, int C, int X, int Y, int W,
                 int H) override {
    char s[40];
    int set = R / CACHE_WAYS;
    int way = R % CACHE_WAYS;
    const CacheLine &line = pe->getCache()->getSets()[set][way];

    switch (context) {
    case CONTEXT_COL_HEADER:
      fl_push_clip(X, Y, W, H);
      fl_draw_box(FL_FLAT_BOX, X, Y, W, H, fl_rgb_color(200, 200, 200));
      fl_color(FL_BLACK);
      switch (C) {
      case 0:
        fl_draw("Set", X, Y, W, H, FL_ALIGN_CENTER);
        break;
      case 1:
        fl_draw("Way", X, Y, W, H, FL_ALIGN_CENTER);
        break;
      case 2:
        fl_draw("Tag", X, Y, W, H, FL_ALIGN_CENTER);
        break;
      case 3:
        fl_draw("Usage", X, Y, W, H, FL_ALIGN_CENTER);
        break;
      default:
        fl_draw(("D" + std::to_string(C - 4)).c_str(), X, Y, W, H,
                FL_ALIGN_CENTER);
        break;
      }
      fl_pop_clip();
      break;
    case CONTEXT_CELL:
      fl_push_clip(X, Y, W, H);
      fl_color(FL_WHITE);
      fl_rectf(X, Y, W, H);
      fl_color(FL_BLACK);
      switch (C) {
      case 0:
        snprintf(s, sizeof(s), "%d", set);
        break;
      case 1:
        snprintf(s, sizeof(s), "%d", way);
        break;
      case 2:
        snprintf(s, sizeof(s), "%d", line.tag);
        break;
      case 3:
        snprintf(s, sizeof(s), "%d", line.usage_count);
        break;
      default:
        snprintf(s, sizeof(s), "%.2f", line.data[C - 4]);
        break;
      }
      fl_draw(s, X + 4, Y, W - 4, H, FL_ALIGN_LEFT);
      fl_rect(X, Y, W, H);
      fl_pop_clip();
      break;
    default:
      break;
    }
  }
};

struct RunData {
    FileLineSelector* inst;
    int* exe_index;
    std::vector<ProcessingElement*>* pes;
};

// ========================================
// STEP sincronizado: estado y funciones
// Ejecuta la misma instrucción (índice) en todos los PEs al mismo tiempo
// ========================================
struct StepState {
    std::vector<std::vector<std::vector<std::string>>> pe_instructions; // [PE][bloque][instr]
    int global_block;       // bloque actual común
    int global_instr;       // índice de instrucción dentro del bloque
    bool initialized;

    StepState() : global_block(0), global_instr(0), initialized(false) {
        pe_instructions.resize(NUM_PE);
    }
};

struct StepData {
    FileLineSelector* inst;
    std::vector<ProcessingElement*>* pes;
};

StepState g_step_state;

void parse_instructions_for_step(FileLineSelector *fls, StepState* state) {
    state->pe_instructions.clear();
    state->pe_instructions.resize(NUM_PE);

    int current_pe = -1;
    std::vector<std::string> file_lines = fls->get_file_lines();

    for (const auto &line : file_lines) {
        if (line.rfind(".PE", 0) == 0) {
            // formato .PE#
            current_pe = std::stoi(line.substr(3));
            if (current_pe >= 0 && current_pe < NUM_PE) {
                state->pe_instructions[current_pe].push_back({});
            }
        } else if (current_pe != -1 && current_pe >= 0 && current_pe < NUM_PE) {
            if (line == "#BREAKPOINT") {
                state->pe_instructions[current_pe].push_back({});
            } else {
                if (state->pe_instructions[current_pe].empty()) {
                    state->pe_instructions[current_pe].push_back({});
                }
                state->pe_instructions[current_pe].back().push_back(line);
            }
        }
    }

    state->global_block = 0;
    state->global_instr = 0;
    state->initialized = true;

    std::cout << "\n🔧 Instrucciones parseadas para STEP:\n";
    for (int i = 0; i < NUM_PE; ++i) {
        std::cout << "  PE" << i << ": " << state->pe_instructions[i].size() << " bloques\n";
    }
    std::cout << std::endl;
}
//
void step(StepData* data) {
    StepState* state = &g_step_state;

    if (!state->initialized) {
        parse_instructions_for_step(data->inst, state);
    }

    bool hayTrabajo = false;
    std::vector<std::thread> threads;

    std::cout << "\n STEP sincronizado: Bloque " << state->global_block
              << ", Instrucción " << state->global_instr << "\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";

    for (int pe = 0; pe < NUM_PE; ++pe) {
        auto &bloques = state->pe_instructions[pe];

        // Si el PE no tiene ese bloque, se salta
        if (state->global_block >= (int)bloques.size()) continue;

        auto &instrucciones = bloques[state->global_block];

        // Si el PE no tiene esa instrucción indexada, se salta
        if (state->global_instr >= (int)instrucciones.size()) continue;

        hayTrabajo = true;
        std::string instr = instrucciones[state->global_instr];

        std::cout << "▶️ [PE" << pe << "] ejecutando: " << instr << std::endl;

        // Ejecutar en paralelo por PE
        threads.emplace_back([pe, instr, data]() {
            (*(data->pes))[pe]->execute(instr);
        });
    }

    // esperar
    for (auto &t : threads) t.join();

    if (!hayTrabajo) {
        std::cout << "✅ No hay más instrucciones pendientes en ninguno de los PEs (STEP terminó).\n";
        return;
    }

    // avanzar índice dentro del bloque
    state->global_instr++;

    // comprobar si ya no hay instrucciones en este bloque para ninguno (entonces avanzar bloque)
    bool anyRemainingInBlock = false;
    for (int pe = 0; pe < NUM_PE; ++pe) {
        auto &bloques = state->pe_instructions[pe];
        if (state->global_block < (int)bloques.size()) {
            if (state->global_instr < (int)bloques[state->global_block].size()) {
                anyRemainingInBlock = true;
                break;
            }
        }
    }

    if (!anyRemainingInBlock) {
        // avanzar a siguiente bloque global
        state->global_block++;
        state->global_instr = 0;
        std::cout << "🔁 Avanzando a bloque global " << state->global_block << "\n";
    }
}

void step_callback(Fl_Widget* widget, void* user_data) {
    StepData* data = static_cast<StepData*>(user_data);
    step(data);
}

void reset_step_callback(Fl_Widget* widget, void* user_data) {
    (void) user_data;
    g_step_state.global_block = 0;
    g_step_state.global_instr = 0;
    g_step_state.initialized = false;
    std::cout << "\n🔄 STEP reiniciado: Bloque 0, Instrucción 0 (re-parse al siguiente STEP)\n\n";
}

// ==========================================
// Callbacks generales
// ==========================================
void on_close(Fl_Widget *, void *) { exit(0); }

void load_memory_cb(Fl_Widget *w, void *data) {
  MemoryTable *mem_tab = static_cast<MemoryTable *>(data);

  const char *filename =
      fl_file_chooser("Seleccionar archivo de memoria", "*.txt", nullptr);
  if (!filename)
    return;

  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "No se pudo abrir el archivo de memoria.\n";
    return;
  }

  std::vector<std::vector<double>> mem_data;
  std::string line;
  while (std::getline(file, line)) {
    std::vector<double> row;
    size_t pos = 0;
    while (pos < line.size()) {
      double val;
      int n = 0;
      if (sscanf(line.c_str() + pos, "%lf%n", &val, &n) == 1) {
        row.push_back(val);
        pos += n;
      } else
        break;
    }
    mem_data.push_back(row);
  }

  file.close();
  mem_tab->set_memory(mem_data);
  std::cout << "Memoria cargada: " << filename << std::endl;
}

//  CALLBACK para cargar instrucciones
void load_instructions_cb(Fl_Widget *w, void *data) {
  FileLineSelector *inst_selector = static_cast<FileLineSelector *>(data);
  inst_selector->load_instructions_file();

  // Mostrar confirmación
  std::cout << "Instrucciones cargadas exitosamente!" << std::endl;

  // Al recargar instrucciones reiniciamos el estado de STEP para que se re-parseen
  g_step_state.initialized = false;
}

void run(FileLineSelector *fls, int &exe_index, std::vector<ProcessingElement*> *pes) {
  std::vector<std::vector<std::vector<std::string>>> pe_instructions(NUM_PE);
  int current_pe = -1;

  std::vector<std::string> file_lines = fls->get_file_lines(); // ← Necesitas get_file_lines() completo
  
  for (const auto &line : file_lines) {
    if (line.rfind(".PE", 0) == 0) { // Si empieza con ".PE"
      current_pe = std::stoi(line.substr(3)); // Obtener el número de PE
      if (current_pe >= 0 && current_pe < NUM_PE) {
        pe_instructions[current_pe].push_back({}); // Crear el primer bloque
      }
    } else if (current_pe != -1 && current_pe >= 0 && current_pe < NUM_PE) {
      if (line == "#BREAKPOINT") {
        // Crear nuevo bloque para el siguiente conjunto de instrucciones
        pe_instructions[current_pe].push_back({});
      } else {
        // Agregar instrucción al bloque actual
        if (pe_instructions[current_pe].empty()) {
          pe_instructions[current_pe].push_back({});
        }
        pe_instructions[current_pe].back().push_back(line);
      }
    }
    std::cout << "[PE_LINE] " << line << " exe_index=" << exe_index << "\n";
  }

  // Print para verificar
  for (int i = 0; i < NUM_PE; i++) {
    std::cout << "\n=== PE" << i << " ===\n";
    for (size_t b = 0; b < pe_instructions[i].size(); ++b) {
      if (pe_instructions[i][b].empty())
        continue;
      std::cout << "  Bloque " << b << ":\n";
      for (const auto &instr : pe_instructions[i][b]) {
        std::cout << "    " << instr << "\n";
      }
    }
  }

  // Verificar que exe_index sea válido para todos los PEs
  std::cout << "\n=== Ejecutando bloque " << exe_index << " ===\n";
  
  bool all_have_block = true;
  for (int i = 0; i < NUM_PE; i++) {
    if (exe_index >= static_cast<int>(pe_instructions[i].size()) || 
        pe_instructions[i][exe_index].empty()) {
      std::cout << "[PE" << i << "] no tiene bloque " << exe_index << "\n";
      all_have_block = false;
    }
  }

  if (!all_have_block) {
    std::cout << " No todos los PEs tienen el bloque " << exe_index << "\n";
    return;
  }

  // Ejecutar los hilos de cada PE en paralelo
  std::vector<std::thread> threads;
  for (int i = 0; i < NUM_PE; i++) {
    threads.emplace_back([&pe_instructions, &pes, i, exe_index]() {
      const auto &block = pe_instructions[i][exe_index];
      std::cout << "[PE" << i << "] ejecutando bloque " << exe_index << ":\n";
      for (const auto &instr : block) {
        std::cout << "  [PE" << i << "] " << instr << "\n";
        (*pes)[i]->execute(instr);
      }
    });
  }

  // Esperar a que todos terminen
  for (auto &t : threads) {
    t.join();
  }

  std::cout << "\n=== Estado de los PEs después del bloque " << exe_index << " ===\n";
  for (int i = 0; i < NUM_PE; i++) {
    std::cout << "\n[PE" << i << "]:\n";
    (*pes)[i]->printStatus();
  }

  std::cout << "\n=== FIN DE EJECUCIÓN DEL BLOQUE " << exe_index << " ===\n\n";
  exe_index++;
}

// corrige el incremento doble: run() ya incrementa exe_index, así que el callback solo llama a run
void run_callback(Fl_Widget* widget, void* user_data) {
    RunData* data = static_cast<RunData*>(user_data);
    run(data->inst, *(data->exe_index), (data->pes));
}

// ==========================================
// Main
// ==========================================
int main() {

  // Inicializar memoria
  Memory *memory = new Memory();

  Interconnect *bus = new Interconnect(memory);

  std::vector<ProcessingElement *> pes;
  for (int i = 0; i < NUM_PE; i++) {
    ProcessingElement *pe = new ProcessingElement(i, bus);
    bus->registerSnoopModule(pe->getSnoop());
    pes.push_back(pe);
  }

  pes[0]->mov(0, 45.5);
  pes[0]->mov(1, 8);
  pes[0]->store(0, 1);
  pes[0]->load(2, 1);

  Fl_Window *win = new Fl_Window(1000, 800, " Visor de PEs");
  Fl_Tabs *tabs = new Fl_Tabs(10, 10, 980, 700);

  // Tab de instrucciones / FileLineSelector
  Fl_Group *grp = new Fl_Group(10, 40, 980, 610, "Instrucciones");
  grp->color(fl_rgb_color(245, 240, 255));

  grp->box(FL_EMBOSSED_BOX);
  FileLineSelector *inst = new FileLineSelector(20, 50, 940, 540, win);
  grp->end();

  // Tabs para PEs
  for (int pe = 0; pe < NUM_PE; pe++) {
    char label[20];
    sprintf(label, "PE %d", pe);
    char *label_copy = strdup(label);
    Fl_Group *grp = new Fl_Group(10, 40, 980, 610, label_copy);
    grp->color(fl_rgb_color(240, 245, 255));

    // Tabla de registros
    RegTable *reg_tab = new RegTable(20, 50, 300, 400, pes[pe]);
    // Tabla de cache
    CacheTable *cache_tab = new CacheTable(300, 50, 700, 600, pes[pe]);
    grp->end();
  }

  // Tab de memoria
  Fl_Group *grp_mem = new Fl_Group(10, 40, 980, 610, "Memoria");
  MemoryTable *mem_tab = new MemoryTable(20, 50, 810, 600);
  grp_mem->end();

  tabs->end();

  // Botón cerrar
  Fl_Button *btn_close = new Fl_Button(850, 660, 120, 40, "Cerrar");
  btn_close->color(fl_rgb_color(255, 180, 180));
  btn_close->callback(on_close);

  // Botón cargar memoria
  Fl_Button *btn_load_mem = new Fl_Button(700, 660, 120, 40, "Cargar Memoria");
  btn_load_mem->color(fl_rgb_color(180, 255, 180));
  btn_load_mem->callback(load_memory_cb, mem_tab);

  // Botón step (ahora conectado a STEP sincronizado)
  Fl_Button *btn_step = new Fl_Button(760, 10, 50, 30, "Step");
  btn_step->color(fl_rgb_color(200, 180, 255));
  StepData* step_data = new StepData{inst, &pes};
  btn_step->callback(step_callback, step_data);

  // Botón BreakPoint / RUN
  Fl_Button *btn_run = new Fl_Button(820, 10, 70, 30, "RUN");
  btn_run->color(fl_rgb_color(200, 180, 255));
  RunData* run_data = new RunData{inst, &exe_index, &pes};
  btn_run->callback(run_callback, run_data); 

  // Botón Reset STEP
  Fl_Button *btn_reset_step = new Fl_Button(890, 10, 70, 30, "Reset");
  btn_reset_step->color(fl_rgb_color(255, 220, 180));
  btn_reset_step->callback(reset_step_callback, nullptr);

  win->end();
  win->show();

  return Fl::run();
}
