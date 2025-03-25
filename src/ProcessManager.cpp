#include "ProcessManager.h"
#include <QDebug>
#include <QDir>
#include <QFile>
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

  // handle filesystem commands internally
  const bool handledInternally = handleFileSystemCommand(program, args);

  if (handledInternally) {
    return; // do not fallback to QProcess if handled internally
  }

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
    if (args.isEmpty()) {
      emit processOutputReady(
          "mkdir: missing operand\nTry 'mkdir --help' for more information.");

      return true;
    }

    // create new directory
    for (const QString &dirName : args) {
      bool success = QDir().mkdir(dirName);

      // emit error on no creation
      if (!success) {
        // error message
        QString errorMessage =
            QString("mkdir: cannot create directory '%1'").arg(dirName);
        emit processOutputReady(
            errorMessage); // send error message to QShellUI.
      }
    }

    // emit newline as output to trigger prompt
    emit processOutputReady("\n");
  }

  // handle create file command
  if (command == "touch") {
    // handle empty args
    if (args.isEmpty()) {
      emit processOutputReady(
          "touch: missing operand\nTry 'touch --help' for more information.");
      return true;
    }

    // create new file
    for (const QString &fileName : args) {
      QFile file(fileName);

      if (file.exists()) {
        continue; // ignore
      }

      if (!file.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QString errorMessage =
            QString("touch: cannot create file '%1'").arg(fileName);
        emit processOutputReady(errorMessage);
      }

      else {
        file.close();
      }
    }

    emit processOutputReady("\n");
    return true;
  }

  return false; // not a filesystem command
}
