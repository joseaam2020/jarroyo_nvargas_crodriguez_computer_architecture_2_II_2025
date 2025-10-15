#include <FL/Fl.H>        // Núcleo del sistema de eventos
#include <FL/Fl_Window.H> // Ventana
#include <FL/Fl_Button.H> // Botón

void on_button_click(Fl_Widget* widget, void*) {
    printf("¡Botón presionado!\n");
}

int main() {
    Fl_Window window(300, 200, "Mi primera ventana FLTK");
    Fl_Button button(100, 80, 100, 40, "Presionar");
    button.callback(on_button_click);

    window.end();
    window.show();
    return Fl::run();
}
