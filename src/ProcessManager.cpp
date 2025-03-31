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

  // Capture error
  connect(process, &QProcess::readyReadStandardError, this, [this]() {
    QString error = process->readAllStandardError();
    emit processErrorReady(error);
  });

  // needed to show prompt even with no content
  connect(
      process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
      this, [this](int /*exitCode*/, QProcess::ExitStatus /*status*/) {
        if (process->readAllStandardOutput().trimmed().isEmpty() &&
            process->readAllStandardError().trimmed().isEmpty()) {
          emit processOutputReady(""); // Trigger prompt manually if no output
        }
      });
}

// Method calls for filesystem specific command handlers
bool ProcessManager::handleFileSystemCommand(const QString &command,
                                             const QStringList &args) {
  if (command == "mkdir")
    return handleMkdir(args);

  if (command == "touch")
    return handleTouch(args);

  if (command == "rmdir")
    return handleRmdir(args);

  if (command == "rm")
    return handleRm(args);

  if (command == "mv")
    return handleMv(args);

  return false; // not a filesystem command
}

// mkdir logic implementation
bool ProcessManager::handleMkdir(const QStringList &args) {
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
      emit processOutputReady(errorMessage); // send error message to QShellUI.
    }
  }

  // emit newline as output to trigger prompt
  emit processOutputReady("\n");
  return true;
}

// touch logic implementation
bool ProcessManager::handleTouch(const QStringList &args) {
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

// rmdir logic implementation
bool ProcessManager::handleRmdir(const QStringList &args) {
  if (args.isEmpty()) {
    emit processOutputReady(
        "rmdir: missing operant\nTry rmdir --help for more information.");
    return true;
  }

  // remove directory if exists and is empty
  for (const QString &dirName : args) {
    // current working directory placeholder
    QDir cwd;

    // send error if directory name does not exists
    if (!cwd.exists(dirName)) {
      // error message
      QString errorMessage =
          QString("rmdir: failed to remove '%1': No such file or directory")
              .arg(dirName);
      emit processOutputReady(errorMessage);
      continue;
    }

    // send error if unable to remove
    if (!cwd.rmdir(dirName)) {
      QString errorMessage =
          QString("rmdir: failed to remove '%1': Directory not empty or "
                  "permission denied")
              .arg(dirName);
      emit processOutputReady(errorMessage);
    }
  }

  emit processOutputReady("\n");

  return true;
}

// rm logic implementation
bool ProcessManager::handleRm(const QStringList &args) {
  // handle empty args
  if (args.isEmpty()) {
    emit processOutputReady(
        "rm: missing operand\nTry 'rm --help' for more information.");
    return true;
  }

  // check for recursive flag in command args
  bool recursive = false;
  QStringList paths;

  for (const QString &arg : args) {
    if (arg == "-r" || arg == "-rf" || arg == "-f" || arg == "-fr") {
      // set recursive flag
      recursive = true;
      continue;
    }

    // build new path without previous flags (-r, -rf, -fr, -f)
    paths.append(arg);
  }

  // handle missing operands
  if (paths.isEmpty()) {
    emit processOutputReady("rm: missing file operand");
    return true;
  }

  // handle file or dir does not exist
  for (const QString &target : paths) {
    // retrieve info about target
    QFileInfo targetInfo(target);

    // handle if target info does not exists
    if (!targetInfo.exists()) {
      // send error message
      QString errorMessage =
          QString("rm: cannot remove '%1': no such file or directory")
              .arg(target);
      emit processOutputReady(errorMessage);
      continue;
    }

    // handle directories
    if (targetInfo.isDir()) {
      // recursive flag check
      if (!recursive) {
        // send error message
        QString errorMessage =
            QString("rm: cannot remove '%1/': Is a directory").arg(target);
        emit processOutputReady(errorMessage);
        continue;
      }

      // remove recursively if possible
      QDir directory(target);
      if (!directory.removeRecursively()) {
        // send error message if not possible
        QString errorMessage =
            QString("rm: failed to remove directory '%1'/").arg(target);
        emit processOutputReady(errorMessage);
      }

    }

    else {
      // handle file removal and failure
      if (!QFile::remove(target)) {
        // send error message on file removal failure
        QString errorMessage = QString("rm: failed to remove '%1'").arg(target);
        emit processOutputReady(errorMessage);
      }
    }
  }

  // trigger prompt
  emit processOutputReady("\n");

  return true;
}

// mv logic implementation
bool ProcessManager::handleMv(const QStringList &args) {
  // handle missing operand
  if (args.isEmpty()) {
    // send error message
    emit processOutputReady(
        "mv: missing file operand\nTry 'mv --help' for more information.");
    return true;
  }

  // handle missing destination file operand
  if (args.size() < 2) {
    QString lastArg = args.isEmpty() ? "" : args.first();
    QString errorMessage =
        QString("mv: missing destination file operand after '%1'\nTry 'mv "
                "--help' for more information.")
            .arg(lastArg);
    emit processOutputReady(errorMessage);
    return true;
  }

  // placeholders
  QString source = args[0];
  QString destination = args[1];
  QFileInfo sourceInfo(source);
  QFileInfo destinationInfo(destination);

  // check source existance
  if (!sourceInfo.exists()) {
    // send error message
    QString errorMessage =
        QString("mv: cannot stat '%1' : No such file or directory").arg(source);
    emit processOutputReady(errorMessage);
    return true;
  }

  // handle move to different destination
  if (destinationInfo.exists() && destinationInfo.isDir()) {
    // build new path destination
    QString finalDest = QDir(destination).filePath(sourceInfo.fileName());

    // attempt moving source to destination - handle move failure
    if (!QFile::rename(source, finalDest)) {
      QString errorMessage =
          QString("mv: failed to move '%1' to '%2'").arg(source, destination);
      emit processOutputReady(errorMessage);
      return true;
    }

  } else {
    // attempt rename source - handle renaming failure
    if (!QFile::rename(source, destination)) {
      // send error message
      QString errorMessage =
          QString("mv: failed to move '%1' to '%2'").arg(source, destination);
      emit processOutputReady(errorMessage);
      return true;
    }
  }

  // trigger prompt
  emit processOutputReady("\n");
  return true;
}
