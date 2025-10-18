#include "file_line_selector.h"
#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Table.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>
#include <FL/Fl_File_Chooser.H>
#include <cstdio>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <cstdlib>

#define NUM_PE 4
#define NUM_REG 8
#define CACHE_SETS 8
#define CACHE_WAYS 2
#define DATA_PER_CACHE 4
#define BLOCKS 128
#define ADDRS_PER_BLOCK 4

float PE_regs[NUM_PE][NUM_REG] = {
    {30.0, 27.8, 82.6, 201.6, 0.0, 8.0, 16.0, 24.0},
    {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0},
    {10, 20, 30, 40, 50, 60, 70, 80},
    {0, 0, 0, 0, 0, 0, 0, 0}
};

struct CacheLine {
    int tag;
    int usage;
    float data[DATA_PER_CACHE];
};

CacheLine PE_cache[NUM_PE][CACHE_SETS][CACHE_WAYS];

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
    void draw_cell(TableContext context, int R, int C, int X, int Y, int W, int H) override {
        char s[40];
        switch (context) {
            case CONTEXT_CELL:
                fl_push_clip(X,Y,W,H);
                fl_color(FL_WHITE); fl_rectf(X,Y,W,H);
                fl_color(FL_BLACK); fl_rect(X,Y,W,H);
                if(C==0) snprintf(s,sizeof(s),"Bloque %d", R);
                else if(R < static_cast<int>(memory_data.size()) && C-1 < static_cast<int>(memory_data[R].size()))
                    snprintf(s,sizeof(s),"%.2f", memory_data[R][C-1]);
                else snprintf(s,sizeof(s),"0.00");
                fl_draw(s, X+4,Y,W-4,H,FL_ALIGN_LEFT);
                fl_pop_clip();
                break;
            default: break;
        }
    }
};

// ==========================================
// Table para Registros
// ==========================================
class RegTable : public Fl_Table {
    int pe_index;

public:
    RegTable(int X, int Y, int W, int H, int pe)
        : Fl_Table(X, Y, W, H), pe_index(pe) {
        rows(NUM_REG);
        cols(2);
        row_header(0);
        col_header(1);
        col_resize(1);
        end();
    }

protected:
    void draw_cell(TableContext context, int R, int C, int X, int Y, int W, int H) override {
        char s[40];
        switch (context) {
            case CONTEXT_COL_HEADER:
                fl_push_clip(X, Y, W, H);
                fl_draw_box(FL_FLAT_BOX, X, Y, W, H, fl_rgb_color(200,200,200));
                fl_color(FL_BLACK);
                fl_draw(C==0?"Registro":"Valor", X,Y,W,H,FL_ALIGN_CENTER);
                fl_pop_clip();
                break;
            case CONTEXT_CELL:
                fl_push_clip(X,Y,W,H);
                fl_color(FL_WHITE); fl_rectf(X,Y,W,H);
                fl_color(FL_BLACK);
                if(C==0) snprintf(s,sizeof(s),"R%d", R);
                else snprintf(s,sizeof(s),"%.4f", PE_regs[pe_index][R]);
                fl_draw(s,X+4,Y,W-4,H,FL_ALIGN_LEFT);
                fl_rect(X,Y,W,H);
                fl_pop_clip();
                break;
            default: break;
        }
    }
};

// ==========================================
// Table para Cache
// ==========================================
class CacheTable : public Fl_Table {
    int pe_index;

public:
    CacheTable(int X, int Y, int W, int H, int pe)
        : Fl_Table(X, Y, W, H), pe_index(pe) {
        rows(CACHE_SETS*CACHE_WAYS);
        cols(4+DATA_PER_CACHE);
        row_header(0);
        col_header(1);
        col_resize(1);
        end();
    }

protected:
    void draw_cell(TableContext context, int R, int C, int X, int Y, int W, int H) override {
        char s[40];
        int set = R / CACHE_WAYS;
        int way = R % CACHE_WAYS;
        CacheLine &line = PE_cache[pe_index][set][way];

        switch(context) {
            case CONTEXT_COL_HEADER:
                fl_push_clip(X,Y,W,H);
                fl_draw_box(FL_FLAT_BOX,X,Y,W,H,fl_rgb_color(200,200,200));
                fl_color(FL_BLACK);
                switch(C){
                    case 0: fl_draw("Set",X,Y,W,H,FL_ALIGN_CENTER); break;
                    case 1: fl_draw("Way",X,Y,W,H,FL_ALIGN_CENTER); break;
                    case 2: fl_draw("Tag",X,Y,W,H,FL_ALIGN_CENTER); break;
                    case 3: fl_draw("Usage",X,Y,W,H,FL_ALIGN_CENTER); break;
                    default: fl_draw(("D"+std::to_string(C-4)).c_str(), X,Y,W,H,FL_ALIGN_CENTER); break;
                }
                fl_pop_clip();
                break;
            case CONTEXT_CELL:
                fl_push_clip(X,Y,W,H);
                fl_color(FL_WHITE); fl_rectf(X,Y,W,H);
                fl_color(FL_BLACK);
                switch(C){
                    case 0: snprintf(s,sizeof(s),"%d", set); break;
                    case 1: snprintf(s,sizeof(s),"%d", way); break;
                    case 2: snprintf(s,sizeof(s),"%d", line.tag); break;
                    case 3: snprintf(s,sizeof(s),"%d", line.usage); break;
                    default: snprintf(s,sizeof(s),"%.2f", line.data[C-4]); break;
                }
                fl_draw(s,X+4,Y,W-4,H,FL_ALIGN_LEFT);
                fl_rect(X,Y,W,H);
                fl_pop_clip();
                break;
            default: break;
        }
    }
};

// ==========================================
// Callbacks
// ==========================================
void on_close(Fl_Widget*, void*) { exit(0); }

void load_memory_cb(Fl_Widget* w, void* data) {
    MemoryTable* mem_tab = static_cast<MemoryTable*>(data);

    const char* filename = fl_file_chooser("Seleccionar archivo de memoria", "*.txt", nullptr);
    if(!filename) return;

    std::ifstream file(filename);
    if(!file.is_open()) {
        std::cerr << "No se pudo abrir el archivo de memoria.\n";
        return;
    }

    std::vector<std::vector<double>> mem_data;
    std::string line;
    while(std::getline(file,line)) {
        std::vector<double> row;
        size_t pos=0;
        while(pos<line.size()) {
            double val;
            int n=0;
            if(sscanf(line.c_str()+pos,"%lf%n",&val,&n)==1) {
                row.push_back(val);
                pos+=n;
            } else break;
        }
        mem_data.push_back(row);
    }

    file.close();
    mem_tab->set_memory(mem_data);
    std::cout << "Memoria cargada: " << filename << std::endl;
}

// NUEVO CALLBACK para cargar instrucciones
void load_instructions_cb(Fl_Widget* w, void* data) {
    FileLineSelector* inst_selector = static_cast<FileLineSelector*>(data);
    inst_selector->load_instructions_file();
    
    // Mostrar confirmación
    std::cout << "Instrucciones cargadas exitosamente!" << std::endl;
}

// ==========================================
// Main
// ==========================================
int main() {
    // Inicializar cache de ejemplo
    for(int pe=0; pe<NUM_PE; pe++)
        for(int s=0; s<CACHE_SETS; s++)
            for(int w=0; w<CACHE_WAYS; w++) {
                PE_cache[pe][s][w].tag = -1;
                PE_cache[pe][s][w].usage = 0;
                for(int d=0; d<DATA_PER_CACHE; d++) PE_cache[pe][s][w].data[d] = 0.0;
            }

    Fl_Window* win = new Fl_Window(1000,800,"🧩 Visor de PEs");
    Fl_Tabs* tabs = new Fl_Tabs(10,10,980,700);

    // Tab de instrucciones
    Fl_Group* grp_inst = new Fl_Group(10,40,980,610,"Instrucciones");
    FileLineSelector* inst = new FileLineSelector(20,50,940,540,win);
    grp_inst->end();

    // Tabs para PEs
    for(int pe=0; pe<NUM_PE; pe++){
        char label[20]; sprintf(label,"PE %d",pe);
        char* label_copy = strdup(label);
        Fl_Group* grp = new Fl_Group(10,40,980,610,label_copy);

        RegTable* reg_tab = new RegTable(20,50,300,400,pe);
        CacheTable* cache_tab = new CacheTable(300,50,700,600,pe);

        grp->end();
    }

    // Tab de memoria
    Fl_Group* grp_mem = new Fl_Group(10,40,980,610,"Memoria");
    MemoryTable* mem_tab = new MemoryTable(20,50,810,600);
    grp_mem->end();

    tabs->end();

    // Botón cerrar
    Fl_Button* btn_close = new Fl_Button(850,660,120,40,"Cerrar");
    btn_close->color(fl_rgb_color(255,180,180));
    btn_close->callback(on_close);

    // Botón cargar memoria
    Fl_Button* btn_load_mem = new Fl_Button(700,660,120,40,"Cargar Memoria");
    btn_load_mem->color(fl_rgb_color(180,255,180));
    btn_load_mem->callback(load_memory_cb, mem_tab);



    win->end();
    win->show();

    return Fl::run();
}