#include "ProcessManager.h"

// Constructor initializes a process
ProcessManager::ProcessManager(QObject *parent) : QObject(parent){
  // Initialize QProcess
  process = new QProcess(this);
}

ProcessManager::~ProcessManager(){
  // cleanign up process
  process->deleteLater();
};

void ProcessManager::startProcess(QString command) {
  // handle command arguments
  QStringList args = command.split(" ", Qt::SkipEmptyParts);

  // handle empty command
  if (args.isEmpty()) {
    return;
  }

  // Run command inside QProcess
  QString program = args.takeFirst(); // get command itself
  process->start(program, args);

  // Capture process output and send it to QShellUI
  connect(process, &QProcess::readyReadStandardOutput, this, [this]() {
    // get output from running command
    QString output = process->readAllStandardOutput();
    
    // send output back to QShellUI
    emit processOutputReady(output);
  });
}


