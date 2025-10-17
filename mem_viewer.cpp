#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Table.H>
#include <FL/Fl_Button.H>
#include <FL/fl_draw.H>
#include <vector>
#include <string>
#include <cstdio>

// =========================================================
// Datos simulados (podrías reemplazar con los reales)
// =========================================================
#define BLOCKS 128
#define ADDRS_PER_BLOCK 4

float memory[BLOCKS][ADDRS_PER_BLOCK] = {
    {15.00, 13.90, 41.30, 100.80},
    {30.00, 27.80, 82.60, 201.60},
    // el resto quedará en 0.00 automáticamente
};

// =========================================================
// Clase personalizada para mostrar la tabla
// =========================================================
class MemoryTable : public Fl_Table {
public:
    MemoryTable(int X, int Y, int W, int H, const char *L = 0)
        : Fl_Table(X, Y, W, H, L) {
        rows(BLOCKS);
        cols(ADDRS_PER_BLOCK + 1); // +1 para el nombre del bloque
        col_header(1);
        row_header(1);
        col_resize(1);
        row_resize(1);
        end();
    }

private:
    void draw_cell(TableContext context, int R, int C, int X, int Y, int W, int H) override {
    // tu código aquí
        static char s[40];
        switch (context) {
            case CONTEXT_STARTPAGE:
                fl_font(FL_COURIER, 12);
                break;
            case CONTEXT_COL_HEADER:
                fl_push_clip(X, Y, W, H);
                fl_draw_box(FL_FLAT_BOX, X, Y, W, H, fl_rgb_color(210, 210, 210));
                fl_color(FL_BLACK);
                if (C == 0) snprintf(s, sizeof(s), "Block");
                else snprintf(s, sizeof(s), "Addr[%d]", (R * ADDRS_PER_BLOCK) + (C - 1));
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

                if (C == 0) snprintf(s, sizeof(s), "Block %d", R);
                else snprintf(s, sizeof(s), "%.2f", memory[R][C - 1]);

                fl_draw(s, X + 4, Y, W - 4, H, FL_ALIGN_LEFT);
                fl_pop_clip();
                break;
            default:
                break;
        }
    }
};

// =========================================================
// Callback para cerrar
// =========================================================
void on_close(Fl_Widget*, void*) {
    exit(0);
}

// =========================================================
// Main
// =========================================================
int main() {
    Fl_Window *window = new Fl_Window(850, 600, "🧩 Visor de Memoria Compartida");
    window->color(fl_rgb_color(245, 245, 250));

    MemoryTable *table = new MemoryTable(20, 20, 810, 500);
    table->when(FL_WHEN_RELEASE);

    Fl_Button *btn_close = new Fl_Button(700, 540, 120, 40, "Cerrar");
    btn_close->color(fl_rgb_color(255, 180, 180));
    btn_close->callback(on_close);

    window->end();
    window->show();

    return Fl::run();
}