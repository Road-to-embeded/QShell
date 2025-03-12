#include "QShellUI.h"

QShellUI::QShellUI(QWidget *parent) : QMainWindow(parent) {
  // WINDOW setup
  setWindowTitle("Main Window");
  resize(800, 600);

  // Config new shell text area
  editor = new QPlainTextEdit(this);
  editor->setReadOnly(false);
  setCentralWidget(editor);

  // Show prompt
  displayShellPrompt();
}

// Destructor
QShellUI::~QShellUI(){};

// Display prompt method
void QShellUI::displayShellPrompt() {
  // Add shell prompt
  editor->appendPlainText("xande@Zero-Quantity:~$ ");
}
