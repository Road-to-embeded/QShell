#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include <QObject>
#include <QProcess>
#include <QString>

/*
 * @brief ProcessManager handles running processes
 *
 * - Captures output.
 * - Emits signal when output is ready.
 * - Connects signal to QShellUI for live output.
 * - Handles Complition.
 * - Kill processes.
 *
 */
class ProcessManager : public QObject {
  Q_OBJECT // for signal and slots

public:
  explicit ProcessManager(QObject *parent = nullptr);
  ~ProcessManager();

  /*
   * @brief Starts new child process
   */
  void startProcess(QString command);

  /*
   * @brief Validates if user input command is valid
   *
   * @param command - User input command
   *
   * @return Bolean value representing the existance of the command.
   */
  bool commandIsValid(QString command);

signals:
  void processOutputReady(QString output);

private:
  QProcess *process;   // Process instance to run commands
  QString command;     // Stores user input command
  QString errorMessage; // Last error message
};

#endif // PROCESS_MANAGER_H
