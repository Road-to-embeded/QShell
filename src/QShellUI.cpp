#include "QShellUI.h"
#include <QProcessEnvironment>
#include <QHostInfo>

QShellUI::QShellUI(QWidget *parent) : QMainWindow(parent) {
  // WINDOW setup
  setWindowTitle("Main Window");
  resize(800, 600);

  // Config new shell text area
  editor = new QPlainTextEdit(this);
  editor->setReadOnly(false);
  setCentralWidget(editor);

  setUsername(); // set username from OS
  setHostname(); // set hostname from OS

  // create prompt
  QString prompt = createPrompt(username, hostname, cwd);

  // Show prompt
  displayShellPrompt(prompt);
}

// Destructor
QShellUI::~QShellUI(){};

// Set username from OS
void QShellUI::setUsername() {
  // OS env
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

  // Prompt username
  username = env.value("USER", env.value("USERNAME", "Unknown User"));
}

// Set hostname from OS
void QShellUI::setHostname() {
  // OS hostname  
  hostname = QHostInfo::localHostName();
}

// Create prompt
QString QShellUI::createPrompt(QString username, QString hostname,
                               QString cwd) {
  // placeholders
  QString shellPrompt =
      QString("%1@%2:%3$ ").arg(username).arg(hostname).arg(cwd);

  return shellPrompt;
}

// Display prompt method
void QShellUI::displayShellPrompt(QString terminalPrompt) {
  // Add shell prompt
  editor->appendPlainText(terminalPrompt);
}
