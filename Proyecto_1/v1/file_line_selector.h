#ifndef FILE_LINE_SELECTOR_H
#define FILE_LINE_SELECTOR_H

#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Window.H>
#include <string>
#include <vector>

class FileLineSelector {
public:
  FileLineSelector(int x, int y, int w, int h, Fl_Window *parent);

  void load_file(); // Llama al file chooser y carga archivo
  const std::vector<std::string> &get_selected_lines() const;

private:
  static void load_button_cb(Fl_Widget *,
                             void *); // Callback para botón de cargar
  static void
  select_line_cb(Fl_Widget *,
                 void *); // Callback para seleccionar/deseleccionar línea
  void display_lines();
  void toggle_line_selection(int index);

  Fl_Window *parent_window;
  Fl_Scroll *scroll;
  Fl_Button *load_button;

  struct LineEntry {
    Fl_Box *line_box;
    Fl_Button *select_button;
    bool selected;
  };

  std::vector<std::string> file_lines;
  std::vector<LineEntry> line_entries;
};

#endif // FILE_LINE_SELECTOR_H
