#include "file_line_selector.h"
#include "interconnect.h"
#include "mem.h"
#include "processing_element.h"
#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Table.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>
#include <cstdio>
#include <string>
#include <vector>

#define NUM_PE 4
#define NUM_REG 8
#define CACHE_SETS 8
#define CACHE_WAYS 2
#define DATA_PER_CACHE 4
#define BLOCKS 128
#define ADDRS_PER_BLOCK 4

float memory[BLOCKS][ADDRS_PER_BLOCK] = {
    {15.00, 13.90, 41.30, 100.80}, {30.00, 27.80, 82.60, 201.60},
    // el resto quedará en 0.00 automáticamente
};

// =========================================================
// Table para Memoria
// =========================================================
class MemoryTable : public Fl_Table {
public:
  MemoryTable(int X, int Y, int W, int H, const char *L = 0)
      : Fl_Table(X, Y, W, H, L) {
    rows(BLOCKS);
    cols(ADDRS_PER_BLOCK + 1); // +1 para el nombre del bloque
    row_height_all(30); // Altura de cada fila (ajústalo a lo que quieras)
    col_width_all(100); // Ancho de cada columna (puedes variarlo)
    col_header(1);
    row_header(1);
    col_resize(1);
    row_resize(1);
    end();
  }

private:
  void draw_cell(TableContext context, int R, int C, int X, int Y, int W,
                 int H) override {
    // tu código aquí
    static char s[40];
    switch (context) {
    case CONTEXT_STARTPAGE:
      fl_font(FL_COURIER, 18);
      break;
    case CONTEXT_COL_HEADER:
      fl_push_clip(X, Y, W, H);
      fl_draw_box(FL_FLAT_BOX, X, Y, W, H, fl_rgb_color(210, 210, 210));
      fl_color(FL_BLACK);
      if (C == 0)
        snprintf(s, sizeof(s), "Block");
      else
        snprintf(s, sizeof(s), "Addr[%d]", (R * ADDRS_PER_BLOCK) + (C - 1));
      fl_draw(s, X, Y, W, H, FL_ALIGN_CENTER);
      fl_pop_clip();
      break;
    case CONTEXT_ROW_HEADER:
      fl_push_clip(X, Y, W, H);
      fl_draw_box(FL_FLAT_BOX, X, Y, W, H, fl_rgb_color(230, 230, 230));
      fl_color(FL_BLACK);
      snprintf(s, sizeof(s), "%d", R);
      fl_draw(s, X, Y, W, H, FL_ALIGN_CENTER);
      fl_pop_clip();
      break;
    case CONTEXT_CELL:
      fl_push_clip(X, Y, W, H);
      fl_color(fl_rgb_color(250, 250, 250));
      fl_rectf(X, Y, W, H);
      fl_color(FL_GRAY);
      fl_rect(X, Y, W, H);
      fl_color(FL_BLACK);

      if (C == 0)
        snprintf(s, sizeof(s), "Block %d", R);
      else
        snprintf(s, sizeof(s), "%.2f", memory[R][C - 1]);

      fl_draw(s, X + 4, Y, W - 4, H, FL_ALIGN_LEFT);
      fl_pop_clip();
      break;
    default:
      break;
    }
  }
};

// ==========================================
// Tabla para Registros
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
// Tabla para Cache
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

// ==========================================
// Main
// ==========================================
void on_close(Fl_Widget *, void *) { exit(0); }

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

  Fl_Window *win = new Fl_Window(1000, 800, "🧩 Visor de PEs");
  Fl_Tabs *tabs = new Fl_Tabs(10, 10, 980, 700);

  // Tab de instrucciones / FileLineSelector
  Fl_Group *grp = new Fl_Group(10, 40, 980, 610, "Instrucciones");
  FileLineSelector *inst = new FileLineSelector(20, 50, 940, 540, win);
  grp->end();

  for (int pe = 0; pe < NUM_PE; pe++) {
    char label[20];
    sprintf(label, "PE %d", pe);
    char *label_copy = strdup(label); // Copia dinámica de la cadena
    Fl_Group *grp = new Fl_Group(10, 40, 980, 610, label_copy);

    // Tabla de registros
    RegTable *reg_tab = new RegTable(20, 50, 300, 400, pes[pe]);
    std::cout << "AQUI!" << std::endl;
    // Tabla de cache
    CacheTable *cache_tab = new CacheTable(300, 50, 700, 600, pes[pe]);
    grp->end();
  }

  grp = new Fl_Group(10, 40, 980, 610, "Memoria");
  MemoryTable *mem_tab = new MemoryTable(20, 50, 810, 600);
  grp->end();

  tabs->end();

  Fl_Button *btn_close = new Fl_Button(850, 660, 120, 40, "Cerrar");
  btn_close->color(fl_rgb_color(255, 180, 180));
  btn_close->callback(on_close);

  win->end();
  win->show();

  return Fl::run();
}
