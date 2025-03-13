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
  // Run command inside QProcess
  process->start(command);

  // Capture process output and send it to QShellUI
  connect(process, &QProcess::readyReadStandardOutput, this, [this]() {
    // get output from running command
    QString output = process->readAllStandardOutput();
    
    // send output back to QShellUI
    emit processOutputReady(output);
  });
}


