#include "file_line_selector.h"
#include <FL/Fl_File_Chooser.H>
#include <fstream>
#include <iostream>

const int LINE_HEIGHT = 30;
const int MARGIN = 10;

struct CallbackData {
  FileLineSelector *selector;
  int index;
};

FileLineSelector::FileLineSelector(int x, int y, int w, int h, Fl_Window *parent)
    : parent_window(parent) {
  // Botón Cargar Archivo
  load_button = new Fl_Button(x + MARGIN, y + MARGIN, 150, 30, "Cargar Archivo");
  load_button->color(fl_rgb_color(180, 255, 180)); // Verde claro
  load_button->callback(load_button_cb, this);

  bp_counter_box = new Fl_Box(x + 300, y + MARGIN, 150, 30, "Breakpoints: 0");
  bp_counter_box->box(FL_FLAT_BOX);
  bp_counter_box->color(fl_rgb_color(240, 240, 240));
  bp_counter_box->labelsize(14);
  bp_counter_box->align(FL_ALIGN_CENTER);




  scroll = new Fl_Scroll(x, y + 50, w, h - 60);
  scroll->box(FL_DOWN_BOX);
}

// Cargar archivo de instrucciones ---
void FileLineSelector::load_instructions_file() {
  const char *filename = fl_file_chooser("Seleccionar archivo de instrucciones", "*.txt", nullptr);
  if (!filename)
    return;

  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "No se pudo abrir el archivo de instrucciones.\n";
    return;
  }

  std::vector<std::string> instruction_lines;
  std::string line;
  
  // Leer y procesar el archivo
  while (std::getline(file, line)) {
    // Limpiar la línea de espacios en blanco
    size_t start = line.find_first_not_of(" \t");
    if (start != std::string::npos) {
      size_t end = line.find_last_not_of(" \t");
      std::string clean_line = line.substr(start, end - start + 1);
      
      // Ignorar líneas vacías pero mantener las que tienen contenido
      if (!clean_line.empty()) {
        instruction_lines.push_back(clean_line);
      }
    }
  }

  file.close();

  // Mostrar resultados en consola
  std::cout << "=== ARCHIVO DE INSTRUCCIONES CARGADO ===" << std::endl;
  std::cout << "Archivo: " << filename << std::endl;
  std::cout << "Total de instrucciones: " << instruction_lines.size() << std::endl;
  std::cout << "Instrucciones:" << std::endl;
  
  for (size_t i = 0; i < instruction_lines.size(); ++i) {
    std::cout << "[" << i << "]\t" << instruction_lines[i] << std::endl;
  }
  std::cout << "=====================================" << std::endl;
}

// --- Función para cargar archivo normal ---
void FileLineSelector::load_file() {
  const char *filename = fl_file_chooser("Seleccionar archivo", "*", nullptr);
  if (!filename)
    return;

  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "No se pudo abrir el archivo.\n";
    return;
  }

  file_lines.clear();

  // Limpiar widgets anteriores
  for (auto &entry : line_entries) {
    if (entry.select_button) {
      CallbackData *data = static_cast<CallbackData *>(entry.select_button->user_data());
      delete data;
    }
    scroll->remove(entry.line_box);
    scroll->remove(entry.select_button);
    delete entry.line_box;
    delete entry.select_button;
  }
  line_entries.clear();

  std::string line;
  while (std::getline(file, line)) {
    file_lines.push_back(line);
  }

  file.close();
  display_lines();
}

// --- Función para cargar archivo de memoria ---
void FileLineSelector::load_memory_file() {
  const char *filename = fl_file_chooser("Seleccionar archivo de memoria", "*.txt", nullptr);
  if (!filename)
    return;

  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "No se pudo abrir el archivo de memoria.\n";
    return;
  }

  memory_lines.clear();
  std::string line;
  while (std::getline(file, line)) {
    memory_lines.push_back(line);
  }

  file.close();

  std::cout << "Archivo de memoria cargado: " << filename << std::endl;
  std::cout << "Total de líneas: " << memory_lines.size() << std::endl;
}

void FileLineSelector::display_lines() {
  // Limpiar widgets anteriores
  for (auto &entry : line_entries) {
    scroll->remove(entry.line_box);
    scroll->remove(entry.select_button);
    delete entry.line_box;
    delete entry.select_button;
  }
  line_entries.clear();

  int y = MARGIN;

  for (size_t i = 0; i < file_lines.size(); ++i) {
    Fl_Button *btn = new Fl_Button(MARGIN, y, 100, LINE_HEIGHT, "Seleccionar");
    btn->color(FL_LIGHT2);
    CallbackData *cb_data = new CallbackData{this, static_cast<int>(i)};
    btn->user_data(cb_data);
    btn->callback(select_line_cb, cb_data);

    Fl_Box *line_box = new Fl_Box(MARGIN + 110, y, scroll->w() - 150,
                                  LINE_HEIGHT, file_lines[i].c_str());
    line_box->box(FL_FLAT_BOX);
    line_box->color(FL_WHITE);
    line_box->labelsize(14);
    line_box->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

    scroll->add(btn);
    scroll->add(line_box);

    line_entries.push_back({line_box, btn, false});
    y += LINE_HEIGHT + 5;
  }

  scroll->redraw();
}

void FileLineSelector::toggle_line_selection(int index) {
  if (index < 0 || index >= static_cast<int>(line_entries.size()))
    return;

  LineEntry &entry = line_entries[index];
  entry.selected = !entry.selected;

  std::string current_text = file_lines[index];

  // Variable estática para recordar el último breakpoint alcanzado
  static int last_breakpoint_index = -1;

  if (entry.selected) {
    // Marcar visualmente la línea seleccionada
    entry.line_box->color(FL_YELLOW);
    entry.select_button->color(FL_GREEN);

    // Agregar el marcador de BREAKPOINT justo al final
    if (current_text.find("#BREAKPOINT") == std::string::npos) {
      // Eliminar espacios al final para evitar repeticiones innecesarias
      while (!current_text.empty() && std::isspace(current_text.back())) {
        current_text.pop_back();
      }

      current_text += "  #BREAKPOINT";
      file_lines[index] = current_text;
      entry.line_box->copy_label(file_lines[index].c_str());
      breakpoint_count++;

      // ---  imprimir instrucciones desde el último breakpoint hasta el actual ---
      int start_index = last_breakpoint_index + 1;
      int end_index = index;

      std::cout << "\n=== Instrucciones desde línea "
                << start_index << " hasta " << end_index << " ===" << std::endl;

      for (int i = start_index; i <= end_index && i < static_cast<int>(file_lines.size()); ++i) {
        std::cout << "[" << i << "] " << file_lines[i] << std::endl;
      }

      std::cout << "=============================================\n" << std::endl;

      // Actualizar el último breakpoint alcanzado
      last_breakpoint_index = end_index;
    }

  } else {
    // Desmarcar visualmente la línea
    entry.line_box->color(FL_WHITE);
    entry.select_button->color(FL_LIGHT2);

    // Eliminar el marcador de BREAKPOINT
    size_t pos = current_text.find("#BREAKPOINT");
    if (pos != std::string::npos) {
      current_text.erase(pos, std::string("#BREAKPOINT").length());
      // Eliminar espacios sobrantes que queden al final
      while (!current_text.empty() && std::isspace(current_text.back())) {
        current_text.pop_back();
      }

      file_lines[index] = current_text;
      entry.line_box->copy_label(file_lines[index].c_str());
      breakpoint_count--;

      std::cout << "\n=== Instrucciones tras quitar BREAKPOINT (línea "
                << index << ") ===" << std::endl;
      for (int i = 0; i <= index && i < static_cast<int>(file_lines.size()); ++i) {
        std::cout << "[" << i << "] " << file_lines[i] << std::endl;
      }
      std::cout << "=============================================\n" << std::endl;
    }
  }

  // Refrescar elementos visuales
  entry.line_box->redraw();
  entry.select_button->redraw();

  // Actualizar contador de breakpoints
  bp_counter_box->label(("Breakpoints: " + std::to_string(breakpoint_count)).c_str());
  bp_counter_box->redraw();

  std::cout << "Breakpoints activos: " << breakpoint_count << std::endl;
}


// --- Callbacks ---
void FileLineSelector::load_button_cb(Fl_Widget *, void *user_data) {
  auto *self = static_cast<FileLineSelector *>(user_data);
  self->load_file();
}

void FileLineSelector::memory_button_cb(Fl_Widget *, void *user_data) {
  auto *self = static_cast<FileLineSelector *>(user_data);
  self->load_memory_file();
}

void FileLineSelector::select_line_cb(Fl_Widget *, void *data) {
  CallbackData *cb_data = static_cast<CallbackData *>(data);
  cb_data->selector->toggle_line_selection(cb_data->index);
}

// --- Get selected lines ---
const std::vector<std::string> &FileLineSelector::get_selected_lines() const {
  static std::vector<std::string> selected;
  selected.clear();

  for (size_t i = 0; i < file_lines.size(); ++i) {
    if (line_entries[i].selected) {
      selected.push_back(file_lines[i]);
    }
  }
  return selected;
}