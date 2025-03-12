#include "QShellUI.h"

QShellUI::QShellUI(QWidget *parent) : QMainWindow(parent) {
  // WINDOW setup
  setWindowTitle("Main Window");
  resize(800, 600);

  // Config new shell text area
  editor = new QPlainTextEdit(this);
  editor->setReadOnly(false);
  setCentralWidget(editor);

  // create prompt
  QString prompt = createPrompt(username, hostname, cwd);

  // Show prompt
  displayShellPrompt(prompt);
}

// Destructor
QShellUI::~QShellUI(){};

// Create prompt 
QString QShellUI::createPrompt(QString username, QString hostname, QString cwd) {
  // placeholders
  QString shellPrompt = QString("%1@%2:%3$ ").arg(username).arg(hostname).arg(cwd);

  return shellPrompt;
}


// Display prompt method
void QShellUI::displayShellPrompt(QString terminalPrompt) {
  // Add shell prompt
  editor->appendPlainText(terminalPrompt);
}
