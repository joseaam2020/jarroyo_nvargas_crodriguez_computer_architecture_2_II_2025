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

FileLineSelector::FileLineSelector(int x, int y, int w, int h,
                                   Fl_Window *parent)
    : parent_window(parent) {
  load_button =
      new Fl_Button(x + MARGIN, y + MARGIN, 150, 30, "Cargar Archivo");
  load_button->callback(load_button_cb, this);

  scroll = new Fl_Scroll(x, y + 50, w, h - 60);
  scroll->box(FL_DOWN_BOX);
}

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

  // Eliminar widgets anteriores
  for (auto &entry : line_entries) {
    if (entry.select_button) {
      CallbackData *data =
          static_cast<CallbackData *>(entry.select_button->user_data());
      delete data;
    }

    scroll->remove(entry.line_box);
    scroll->remove(entry.select_button);
    delete entry.line_box;
    delete entry.select_button;
  }
  line_entries.clear();
  line_entries.clear();

  std::string line;
  while (std::getline(file, line)) {
    file_lines.push_back(line);
  }

  file.close();
  display_lines();
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
  std::cout << "toggle_line_selection index: " << index << std::endl;

  if (index < 0 || index >= static_cast<int>(line_entries.size())) {
    std::cout << "Invalid index!" << std::endl;
    return;
  }

  LineEntry &entry = line_entries[index];
  if (!entry.line_box || !entry.select_button) {
    std::cout << "Invalid pointers!" << std::endl;
    return;
  }

  entry.selected = !entry.selected;

  if (entry.selected) {
    entry.line_box->color(FL_YELLOW);
    entry.select_button->color(FL_GREEN);
  } else {
    entry.line_box->color(FL_WHITE);
    entry.select_button->color(FL_LIGHT2);
  }

  entry.line_box->redraw();
  entry.select_button->redraw();
}

// --- Callbacks ---

void FileLineSelector::load_button_cb(Fl_Widget *, void *user_data) {
  auto *self = static_cast<FileLineSelector *>(user_data);
  self->load_file();
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
