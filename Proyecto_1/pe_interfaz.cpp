#include "cache.h"
#include "file_line_selector.h"
#include "interconnect.h"
#include "mem.h"
#include "mesi_state.h"
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
#include <iterator>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#define NUM_PE 4
#define NUM_REG 8
#define CACHE_SETS 8
#define CACHE_WAYS 2
#define DATA_PER_CACHE 4
#define BLOCKS 128
#define ADDRS_PER_BLOCK 4

std::vector<int> *exe_index = new std::vector<int>(NUM_PE);
std::vector<int> *pcs = new std::vector<int>(NUM_PE);

// ==========================================
// Table para Memoria
// ==========================================
class MemoryTable : public Fl_Table {
  Memory *memory;

public:
  MemoryTable(int X, int Y, int W, int H, Memory *memory, const char *L = 0)
      : Fl_Table(X, Y, W, H, L), memory(memory) {
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

  void set_memory(Memory *memory) {
    this->memory = memory;
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
      if (C == 0) {
        snprintf(s, sizeof(s), "Bloque %d", R);
      } else {
        int index = R * ADDRS_PER_BLOCK + (C - 1);
        if (index < static_cast<int>(memory->getStorage().size()))
          snprintf(s, sizeof(s), "%.2f", memory->getStorage()[index]);
        else
          snprintf(s, sizeof(s), "0.00");
      }

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

  void setProcessingElement(ProcessingElement *pe) {
    this->pe = pe;
    redraw();
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
    cols(5 +
         DATA_PER_CACHE); // Set, Way, Tag, Usage, Data0, Data1, Data2, Data3
    row_header(0);
    col_header(1);
    col_resize(1);
    end();
  }

  void setProcessingElement(ProcessingElement *pe) {
    this->pe = pe;
    redraw();
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
      case 4:

      default:
        fl_draw(("D" + std::to_string(C - 5)).c_str(), X, Y, W, H,
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
      case 4:
        snprintf(s, sizeof(s), "%s", mesiStateToString(line.state));
        break;
      default:
        snprintf(s, sizeof(s), "%.2f", line.data[C - 5]);
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

// ==========================================
// Table para Estadísticas de Cache
// ==========================================
class CacheStatsTable : public Fl_Table {
  ProcessingElement *pe;

public:
  CacheStatsTable(int X, int Y, int W, int H, ProcessingElement *pe)
      : Fl_Table(X, Y, W, H), pe(pe) {
    rows(6);       // 6 filas de estadísticas
    cols(2);       // Nombre y Valor
    row_header(0); // Sin header de filas
    col_header(1); // CON header de columnas (esto es clave)
    col_resize(1);
    col_width(0, 180);
    col_width(1, 140);
    row_height_all(35);
    end();
  }

  void setProcessingElement(ProcessingElement *pe) {
    this->pe = pe;
    redraw();
  }

private:
  void draw_cell(TableContext context, int R, int C, int X, int Y, int W,
                 int H) override {
    char s[100];

    switch (context) {
    case CONTEXT_COL_HEADER:
      // Dibujar el header de las columnas (igual que CacheTable)
      fl_push_clip(X, Y, W, H);
      fl_draw_box(FL_FLAT_BOX, X, Y, W, H, fl_rgb_color(200, 200, 200));
      fl_color(FL_BLACK);
      fl_font(FL_HELVETICA_BOLD, 12);

      if (C == 0) {
        fl_draw("Métrica", X, Y, W, H, FL_ALIGN_CENTER);
      } else {
        fl_draw("Valor", X, Y, W, H, FL_ALIGN_CENTER);
      }

      fl_pop_clip();
      break;

    case CONTEXT_CELL:
      fl_push_clip(X, Y, W, H);

      // Color de fondo blanco (como CacheTable)
      fl_color(FL_WHITE);
      fl_rectf(X, Y, W, H);

      fl_color(FL_BLACK);
      fl_rect(X, Y, W, H);

      if (C == 0) {
        // Columna de nombres de métricas
        fl_font(FL_HELVETICA, 12);
        switch (R) {
        case 0:
          snprintf(s, sizeof(s), "Cache Misses");
          break;
        case 1:
          snprintf(s, sizeof(s), "Cache Hits");
          break;
        case 2:
          snprintf(s, sizeof(s), "Lecturas");
          break;
        case 3:
          snprintf(s, sizeof(s), "Escrituras");
          break;
        case 4:
          snprintf(s, sizeof(s), "Invalidaciones");
          break;
        case 5:
          snprintf(s, sizeof(s), "Trafico Bus");
          break;
        }
      } else {
        // Columna de valores
        fl_font(FL_HELVETICA, 12);
        CacheStats stats = pe->getCache()->getStats();
        switch (R) {
        case 0:
          snprintf(s, sizeof(s), "%d", stats.cache_misses);
          break;
        case 1:
          snprintf(s, sizeof(s), "%d", stats.cache_hits);
          break;
        case 2:
          snprintf(s, sizeof(s), "%d", stats.reads);
          break;
        case 3:
          snprintf(s, sizeof(s), "%d", stats.writes);
          break;
        case 4:
          snprintf(s, sizeof(s), "%d", stats.invalidations);
          break;
        case 5:
          snprintf(s, sizeof(s), "%d", stats.bus_traffic);
          break;
        }
      }

      fl_draw(s, X + 4, Y, W - 4, H, FL_ALIGN_LEFT);
      fl_pop_clip();
      break;

    default:
      break;
    }
  }
};

// Estructura para pasar la memoria y la tabla
struct MemoryData {
  MemoryTable *mem_tab;
  Memory *memory;
};

struct RunData {
  FileLineSelector *inst;
  std::vector<int> *exe_index;
  std::vector<int> *pcs;
  std::vector<ProcessingElement *> *pes;
};

struct ProgramData {
  struct RunData *run_data;
  Memory *memory;
  Interconnect *interconnect;
  MemoryTable *mem_tab;
  std::vector<RegTable *> reg_tabs;
  std::vector<CacheTable *> cache_tabs;
  std::vector<CacheStatsTable *> stats_tabs;
};

void step(FileLineSelector *fls, std::vector<int> *exe_index,
          std::vector<int> *pcs, std::vector<ProcessingElement *> *pes) {

  std::vector<std::vector<std::vector<std::string>>> pe_instructions(NUM_PE);
  int current_pe = -1;

  std::vector<std::string> file_lines = fls->get_file_lines();

  // Formar bloques segun los breaks
  for (const auto &line : file_lines) {
    if (line.rfind(".PE", 0) == 0) {
      current_pe = std::stoi(line.substr(3));
      if (current_pe >= 0 && current_pe < NUM_PE)
        pe_instructions[current_pe].push_back({});
    } else if (current_pe != -1 && current_pe >= 0 && current_pe < NUM_PE) {
      if (line == "#BREAKPOINT")
        pe_instructions[current_pe].push_back({});
      else {
        if (pe_instructions[current_pe].empty())
          pe_instructions[current_pe].push_back({});
        pe_instructions[current_pe].back().push_back(line);
      }
    }
  }

  // Ejecutar un paso por PE (en paralelo)
  std::vector<std::thread> threads;
  for (int i = 0; i < NUM_PE; i++) {
    threads.emplace_back([&, i]() {
      if ((*exe_index)[i] >= static_cast<int>(pe_instructions[i].size()) ||
          pe_instructions[i][(*exe_index)[i]]
              .empty()) { // Si tiene bloque no tiene bloque
        std::cout << "[PE" << i << "] no tiene bloque " << (*exe_index)[i]
                  << "\n";
        return;
      }

      int block_index = (*exe_index)[i];

      // Evitar acceso fuera de rango
      if ((*pcs)[i] >= (int)pe_instructions[i][block_index].size()) {
        std::cout << "[PE" << i << "] Fin del bloque " << block_index << "\n";
        (*exe_index)[i]++;
        (*pcs)[i] = 0;
        return;
      }

      // Mapa de etiquetas
      std::unordered_map<std::string, std::pair<int, int>> label_map;
      for (int u = 0; u < (int)pe_instructions[i].size(); ++u)
        for (int v = 0; v < (int)pe_instructions[i][u].size(); ++v)
          if (!pe_instructions[i][u][v].empty() &&
              pe_instructions[i][u][v][0] == '_')
            label_map[pe_instructions[i][u][v]] = {u, v};

      const std::string &instr = pe_instructions[i][block_index][(*pcs)[i]];

      if (instr.empty() || instr[0] == '_') {
        (*pcs)[i]++;
        return;
      }

      std::istringstream iss(instr);
      std::string opcode;
      iss >> opcode;

      if (opcode == "jnz") {
        std::string label;
        iss >> label;
        if ((*pes)[i]->getRegisters()[0] != 0) {
          auto it = label_map.find(label);
          if (it != label_map.end()) {
            (*exe_index)[i] = it->second.first;
            (*pcs)[i] = it->second.second;
            std::cout << "[PE" << i << "] Salto a " << label << " (bloque "
                      << (*exe_index)[i] << ", pc " << (*pcs)[i] << ")\n";
            return;
          } else {
            std::cerr << "[PE" << i << "] Etiqueta no encontrada: " << label
                      << "\n";
          }
        }
      } else {
        (*pes)[i]->execute(instr);
      }

      (*pcs)[i]++;
    });
  }

  for (auto &t : threads)
    t.join();

  for (int i = 0; i < NUM_PE; i++) {
    std::cout << "\n[PE" << i << "] Step ejecutado. PC=" << (*pcs)[i]
              << ", Bloque=" << (*exe_index)[i] << "\n";
    (*pes)[i]->printStatus();
  }
};

void step_callback(Fl_Widget *widget, void *user_data) {
  std::cout << "HACIENDO STEP!" << std::endl;
  RunData *data = static_cast<RunData *>(user_data);
  step(data->inst, (data->exe_index), (data->pcs), (data->pes));
}

// ==========================================
// Callbacks generales
// ==========================================
void on_close(Fl_Widget *, void *) { exit(0); }

void load_memory_cb(Fl_Widget *w, void *data) {
  MemoryData *mem_data = static_cast<MemoryData *>(data);
  MemoryTable *mem_tab = mem_data->mem_tab;
  Memory *memory = mem_data->memory;

  const char *filename =
      fl_file_chooser("Seleccionar archivo de memoria", "*.txt", nullptr);
  if (!filename)
    return;

  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "No se pudo abrir el archivo de memoria.\n";
    return;
  }

  // Vector para los valores leídos
  std::vector<std::vector<double>> mem_values;
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
    if (!row.empty())
      mem_values.push_back(
          row); // Se agrega la fila completa al vector principal
  }

  file.close();
  // mem_tab->set_memory(mem_values); // Actualizar la tabla con los valores
  // leídos

  // Llenar la memoria real
  int address = 0; // Dirección inicial
  for (const auto &row : mem_values) {
    for (double val : row) {
      memory->initialize(address, val); // Escribe el valor en memoria real
      address += 8;                     // Avanza 8 bytes
    }
  }
  std::cout << "Memoria cargada: " << filename << std::endl;
  memory->printMemory();
}

//  CALLBACK para cargar instrucciones
void load_instructions_cb(Fl_Widget *w, void *data) {
  FileLineSelector *inst_selector = static_cast<FileLineSelector *>(data);
  inst_selector->load_instructions_file();

  // Mostrar confirmación
  std::cout << "Instrucciones cargadas exitosamente!" << std::endl;
}

void run(FileLineSelector *fls, std::vector<int> *exe_index,
         std::vector<int> *pcs, std::vector<ProcessingElement *> *pes) {
  std::vector<std::vector<std::vector<std::string>>> pe_instructions(NUM_PE);
  int current_pe = -1;

  std::vector<std::string> file_lines =
      fls->get_file_lines(); // ← Necesitas get_file_lines() completo

  for (const auto &line : file_lines) {
    if (line.rfind(".PE", 0) == 0) {          // Si empieza con ".PE"
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
  }

  // Print para verificar
  for (int i = 0; i < NUM_PE; i++) {
    std::cout << "\n=== PE" << i << " ===\n";
    std::cout << "[PE_LINE] " << i << " (*exe_index)[i]=" << (*exe_index)[i]
              << "\n";
    for (size_t b = 0; b < pe_instructions[i].size(); ++b) {
      if (pe_instructions[i][b].empty())
        continue;
      std::cout << "  Bloque " << b << ":\n";
      for (const auto &instr : pe_instructions[i][b]) {
        std::cout << "    " << instr << "\n";
      }
    }
  }

  // Ejecutar los hilos de cada PE en paralelo
  std::vector<std::thread> threads;
  for (int i = 0; i < NUM_PE; i++) {
    threads.emplace_back([&pe_instructions, &pes, i, exe_index, pcs]() {
      if ((*exe_index)[i] >= static_cast<int>(pe_instructions[i].size()) ||
          pe_instructions[i][(*exe_index)[i]]
              .empty()) { // Si tiene bloque no tiene bloque
        std::cout << "[PE" << i << "] no tiene bloque " << (*exe_index)[i]
                  << "\n";
        return;
      }

      // 1. Construir el mapa de etiquetas para este PE
      std::unordered_map<std::string, std::pair<int, int>> label_map;

      for (int u = 0; u < (int)pe_instructions[i].size(); ++u) {
        for (int v = 0; v < (int)pe_instructions[i][u].size(); ++v) {
          const std::string &line = pe_instructions[i][u][v];
          if (!line.empty() && line[0] == '_') {
            label_map[line] = {u, v};
          }
        }
      }

      // 2. Ejecutar desde exe_index y dentro del bloque
      int block_index = (*exe_index)[i];

      while ((*pcs)[i] <
             (int)pe_instructions[i][block_index]
                 .size()) { // Mientras el pc sea menos al tamanio del bloque
        const std::string &instr = pe_instructions[i][block_index][(*pcs)[i]];

        if (instr.empty() || instr[0] == '_') {
          (*pcs)[i]++;
          continue;
        }

        std::istringstream iss(instr);
        std::string opcode;
        iss >> opcode;

        if (opcode == "jnz") {
          std::string label;
          iss >> label;

          if ((*pes)[i]->getRegisters()[0] != 0) { // ejemplo con R1
            auto it = label_map.find(label);
            if (it != label_map.end()) {
              block_index = it->second.first;
              (*pcs)[i] = it->second.second;
              std::cout << "Salto a Bloque: " << block_index
                        << ", Pc: " << (*pcs)[i] << std::endl;

              continue; // saltar a la nueva instrucción
            } else {
              std::cerr << "[PE" << i << "]  Etiqueta no encontrada: " << label
                        << std::endl;
            }
          }
        } else {
          (*pes)[i]->execute(instr);
        }

        (*pcs)[i]++;
      }

      (*exe_index)[i] = ++block_index;
      (*pcs)[i] = 0;
    });
  }

  // Esperar a que todos terminen
  for (auto &t : threads) {
    t.join();
  }

  for (int i = 0; i < NUM_PE; i++) {
    std::cout << "\n[PE" << i << "]:\n";
    (*pes)[i]->printStatus();
    std::cout << "\n=== FIN DE EJECUCIÓN DEL BLOQUE " << (*exe_index)[i]
              << " ===\n\n";
  }
}

// corrige el incremento doble: run() ya incrementa exe_index, así que el
// callback solo llama a run
void run_callback(Fl_Widget *widget, void *user_data) {
  RunData *data = static_cast<RunData *>(user_data);
  run(data->inst, (data->exe_index), (data->pcs), (data->pes));
}

void reset_callback(Fl_Widget *widget, void *user_data) {
  ProgramData *ctx = static_cast<ProgramData *>(user_data);
  if (!ctx || !ctx->run_data)
    return;

  // --- 1. Borrar los objetos viejos ---
  RunData *rd = ctx->run_data;

  // Borrar los PEs
  if (rd->pes) {
    for (auto *pe : *(rd->pes))
      delete pe;
    rd->pes->clear();
  }

  for (int i = 0; i < NUM_PE; i++) {
    ProcessingElement *new_pe = new ProcessingElement(i, ctx->interconnect);
    ctx->interconnect->registerSnoopModule(new_pe->getSnoop());
    rd->pes->push_back(new_pe);
  }

  // --- 3. Actualizar los widgets ---

  for (int i = 0; i < NUM_PE; i++) {
    ctx->reg_tabs[i]->setProcessingElement((*(rd->pes))[i]);
    ctx->cache_tabs[i]->setProcessingElement((*(rd->pes))[i]);
    ctx->stats_tabs[i]->setProcessingElement((*(rd->pes))[i]);
  }

  // --- 4. Resetear contadores de ejecución ---
  std::fill(rd->exe_index->begin(), rd->exe_index->end(), 0);
  std::fill(rd->pcs->begin(), rd->pcs->end(), 0);

  std::cout << "✅ Sistema completamente reiniciado.\n";
}

// ==========================================
// Main
// ==========================================
int main() {

  // Inicializar memoria
  Memory *memory = new Memory();

  Interconnect *bus = new Interconnect(memory);

  ProgramData *data = new ProgramData();
  data->memory = memory;
  data->interconnect = bus;

  std::vector<ProcessingElement *> pes;
  for (int i = 0; i < NUM_PE; i++) {
    ProcessingElement *pe = new ProcessingElement(i, bus);
    bus->registerSnoopModule(pe->getSnoop());
    pes.push_back(pe);
  }

  Fl_Window *win = new Fl_Window(1190, 800, " Visor de PEs");
  Fl_Tabs *tabs = new Fl_Tabs(10, 10, 1150, 700);

  // Tab de instrucciones / FileLineSelector
  Fl_Group *grp = new Fl_Group(10, 40, 1050, 610, "Instrucciones");
  grp->color(fl_rgb_color(245, 240, 255));

  grp->box(FL_EMBOSSED_BOX);
  FileLineSelector *inst = new FileLineSelector(20, 50, 990, 540, win);
  grp->end();

  // Tabs para PEs
  for (int pe = 0; pe < NUM_PE; pe++) {
    char label[20];
    sprintf(label, "PE %d", pe);
    char *label_copy = strdup(label);
    Fl_Group *grp = new Fl_Group(10, 40, 980, 610, label_copy);
    grp->color(fl_rgb_color(240, 245, 255));

    // Tabla de registros
    RegTable *reg_tab = new RegTable(20, 50, 300, 270, pes[pe]);

    // Tabla de stats de PE
    CacheStatsTable *stats_tab =
        new CacheStatsTable(20, 330, 350, 220, pes[pe]);

    // Tabla de cache
    CacheTable *cache_tab = new CacheTable(380, 50, 780, 580, pes[pe]);

    // Se guarda en data
    data->reg_tabs.push_back(reg_tab);
    data->cache_tabs.push_back(cache_tab);
    data->stats_tabs.push_back(stats_tab);

    grp->end();
  }

  // Tab de memoria
  Fl_Group *grp_mem = new Fl_Group(10, 40, 980, 610, "Memoria");
  MemoryTable *mem_tab = new MemoryTable(20, 50, 810, 600, memory);
  grp_mem->end();

  tabs->end();
  data->mem_tab = mem_tab; // guardar en ProgramData

  // Botón cerrar
  Fl_Button *btn_close = new Fl_Button(850, 660, 120, 40, "Cerrar");
  btn_close->color(fl_rgb_color(255, 180, 180));
  btn_close->callback(on_close);

  // Objeto MemoryData, inicializando los punteros mem_tab y memory
  // Empaqueta los punteros para que el callback tenga acceso a eĺ
  MemoryData *mem_data = new MemoryData{mem_tab, memory};

  // Botón cargar memoria ***
  Fl_Button *btn_load_mem = new Fl_Button(700, 660, 120, 40, "Cargar Memoria");
  btn_load_mem->color(fl_rgb_color(180, 255, 180));
  btn_load_mem->callback(load_memory_cb, mem_data); // Le paso el paquete

  RunData *run_data = new RunData{inst, exe_index, pcs, &pes};
  data->run_data = run_data;

  // Botón step (ahora conectado a STEP sincronizado)
  Fl_Button *btn_step = new Fl_Button(760, 10, 50, 30, "Step");
  btn_step->color(fl_rgb_color(200, 180, 255));
  btn_step->callback(step_callback, run_data);

  // Botón BreakPoint / RUN
  Fl_Button *btn_run = new Fl_Button(820, 10, 70, 30, "RUN");
  btn_run->color(fl_rgb_color(200, 180, 255));
  btn_run->callback(run_callback, run_data);

  // Botón Reset STEP
  Fl_Button *btn_reset_step = new Fl_Button(890, 10, 70, 30, "Reset");
  btn_reset_step->color(fl_rgb_color(255, 220, 180));
  btn_reset_step->callback(reset_callback, data);

  win->end();
  win->show();

  return Fl::run();
}
