#include "file_line_selector.h"
#include <FL/Fl_File_Chooser.H>
#include <fstream>
#include <iostream>
#include <algorithm> 
#include <cctype>

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

  while (std::getline(file, line)) {
    // PASO 1: Eliminar TODOS los caracteres de control y no imprimibles
    std::string cleaned;
    for (unsigned char c : line) {
      // Solo mantener caracteres imprimibles ASCII (32-126) y espacios/tabs
      if ((c >= 32 && c <= 126) || c == '\t') {
        cleaned += c;
      }
    }
    line = cleaned;

    // PASO 2: Eliminar espacios iniciales y finales
    size_t start = line.find_first_not_of(" \t");
    if (start == std::string::npos) continue; // línea vacía
    
    size_t end = line.find_last_not_of(" \t");
    line = line.substr(start, end - start + 1);

    if (!line.empty()) {
      instruction_lines.push_back(line);
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
    // Limpieza de caracteres no imprimibles (IGUAL QUE EN load_instructions_file)
    line.erase(std::remove_if(line.begin(), line.end(),
        [](unsigned char c) {
          return (c < 32 && c != 9) || c == 127 || c == '\r' || c == '\n' || c == '\t';
        }),
        line.end());

    // Eliminar espacios iniciales y finales
    line.erase(line.begin(), std::find_if(line.begin(), line.end(),
        [](unsigned char ch) { return !std::isspace(ch); }));
    line.erase(std::find_if(line.rbegin(), line.rend(),
        [](unsigned char ch) { return !std::isspace(ch); }).base(), line.end());

    if (!line.empty()) {
      file_lines.push_back(line);
    }
  }

  file.close();
  display_lines();
  
  // Debug: imprimir las líneas cargadas
  std::cout << "=== ARCHIVO CARGADO ===" << std::endl;
  std::cout << "Total de líneas: " << file_lines.size() << std::endl;
  for (size_t i = 0; i < file_lines.size(); ++i) {
    std::cout << "[" << i << "] " << file_lines[i] << std::endl;
  }
  std::cout << "======================" << std::endl;
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
  if (index < 0 || index >= static_cast<int>(file_lines.size()))
    return;

  // Variable estática para recordar el último breakpoint alcanzado
  static int last_breakpoint_index = -1;

  std::string current_line = file_lines[index];
  
  // Verificar si ya hay un breakpoint después de esta línea
  bool has_breakpoint = (index + 1 < static_cast<int>(file_lines.size()) &&
                         file_lines[index + 1].find("#BREAKPOINT") != std::string::npos);

  if (!has_breakpoint) {
    // AGREGAR BREAKPOINT
    file_lines.insert(file_lines.begin() + index + 1, "#BREAKPOINT");
    breakpoint_count++;

    // Imprimir instrucciones desde el último breakpoint hasta el actual
    int start_index = last_breakpoint_index + 1;
    int end_index = index + 1; // ahora el breakpoint está en index+1

    std::cout << "\n=== Instrucciones desde línea "
              << start_index << " hasta " << end_index << " ===" << std::endl;

    for (int i = start_index; i <= end_index && i < static_cast<int>(file_lines.size()); ++i) {
      std::cout << "[" << i << "] " << file_lines[i] << std::endl;
    }
    std::cout << "=============================================\n" << std::endl;

    last_breakpoint_index = end_index;

  } else {
    // ELIMINAR BREAKPOINT
    file_lines.erase(file_lines.begin() + index + 1);
    breakpoint_count--;

    std::cout << "\n=== BREAKPOINT removido después de línea " << index << " ===" << std::endl;
  }

  // Debug: mostrar todas las líneas
  std::cout << "\n=== Estado actual de file_lines ===" << std::endl;
  for (size_t i = 0; i < file_lines.size(); ++i) {
    std::cout << "[" << i << "] " << file_lines[i] << std::endl;
  }
  std::cout << "====================================\n" << std::endl;

  // IMPORTANTE: Re-crear toda la visualización
  display_lines();

  for (size_t i = 0; i < file_lines.size(); ++i) {
    if (i + 1 < file_lines.size() && 
        file_lines[i + 1].find("#BREAKPOINT") != std::string::npos &&
        i < line_entries.size()) {
      // Esta línea tiene un breakpoint después, colorearla
      line_entries[i].line_box->color(FL_YELLOW);
      line_entries[i].select_button->color(FL_GREEN);
      line_entries[i].select_button->copy_label("Quitar");
      line_entries[i].selected = true;
    }
  }
  
  scroll->redraw();
  // Actualizar contador de breakpoints
  std::string bp_label = "Breakpoints: " + std::to_string(breakpoint_count);
  bp_counter_box->copy_label(bp_label.c_str());
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

const std::vector<std::string> &FileLineSelector::get_file_lines() const {
  static std::vector<std::string> selected;
  selected.clear();

  for (size_t i = 0; i < file_lines.size(); ++i) {
      selected.push_back(file_lines[i]);
  
  }
  return selected;

}