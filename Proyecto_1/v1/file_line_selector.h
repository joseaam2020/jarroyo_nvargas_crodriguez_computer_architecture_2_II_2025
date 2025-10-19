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

  void load_file(); 
  void load_memory_file(); 
  void load_instructions_file(); 

  const std::vector<std::string> &get_selected_lines() const;

private:
  // --- Callbacks ---
  static void load_button_cb(Fl_Widget *, void *);
  static void memory_button_cb(Fl_Widget *, void *); 
  static void select_line_cb(Fl_Widget *, void *);

  // --- Funciones internas ---
  void display_lines();
  void toggle_line_selection(int index);

  // --- Widgets ---
  Fl_Window *parent_window;
  Fl_Scroll *scroll;
  Fl_Button *load_button;
  Fl_Button *memory_button; 

  // --- Datos ---
  struct LineEntry {
    Fl_Box *line_box;
    Fl_Button *select_button;
    bool selected;
  };

  std::vector<std::string> file_lines;
  std::vector<std::string> memory_lines; 
  std::vector<LineEntry> line_entries;
};

#endif // FILE_LINE_SELECTOR_H