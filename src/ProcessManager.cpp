#include "ProcessManager.h"
#include <QDebug>
#include <QStandardPaths>

// Constructor initializes a process
ProcessManager::ProcessManager(QObject *parent) : QObject(parent) {
  // Initialize QProcess
  process = new QProcess(this);
}

ProcessManager::~ProcessManager() {
  // cleanign up process
  process->deleteLater();
};

bool ProcessManager::commandIsValid(const QString command) {
  bool commandFound = !QStandardPaths::findExecutable(command).isEmpty();

  return commandFound;
}

void ProcessManager::startProcess(QString command) {
  // handle command arguments
  QStringList args = command.split(" ", Qt::SkipEmptyParts);

  // handle empty command
  if (args.isEmpty()) {
    return;
  }

  // get command
  QString program = args.takeFirst();

  // validate command
  bool validCommand = commandIsValid(program);
  if (!validCommand) {
    // error message
    errorMessage = "Error: Command '" + program + "' not found.\n";

    // send error message as ouput
    emit processOutputReady(errorMessage);

    return;
  }

  // clear last error message if none detected
  errorMessage.clear();

  // Run command inside QProcess
  process->start(program, args);

  // Capture process output and send it to QShellUI
  connect(process, &QProcess::readyReadStandardOutput, this, [this]() {
    // get output from running command
    QString output = process->readAllStandardOutput();

    // send output back to QShellUI
    emit processOutputReady(output);
  });
}

// Method falls back to QProcess if command not recognized.
bool ProcessManager::handleFileSystemCommand(const QString &command,
                                             const QStringList &args) {
  // handle mdkir
  if (command == "mkdir") {
    // handle empty args
    if (args.isEmpy()) {
      emit processOutputReady(
          "mkdir: missing operand<br>Try 'mkdir --help' for more information.");

      return true;
    }
  }

  return false; // not a filesystem command
}
