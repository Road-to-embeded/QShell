#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>

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

      public : explicit ProcessManager(QObject *parent = nullptr);
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

/**
 * @brief Handles internal file system commands like mkdir, touch, etc.
 * 
 * This function intercepts commands and performs the appropriate file system 
 * operations using Qt APIs instead of external processes.
 *
 * @param command The base command (e.g., "mkdir").
 * @param args The arguments passed to the command.
 * @return true if the command was handled internally; false otherwise.
 */
bool handleFileSystemCommand(const QString &command, const QStringList &args);


/**
 * @brief Handles the 'mkdir' command to create one or more directories.
 * 
 * @param args List of directory names to create.
 * @return true if the command was handled internally, false otherwise.
 */
bool handleMkdir(const QStringList &args);

/**
 * @brief Handles the 'touch' command to create empty files or update timestamps.
 * 
 * @param args List of filenames to create.
 * @return true if the command was handled internally, false otherwise.
 */
bool handleTouch(const QStringList &args);

/**
 * @brief Handles the 'rmdir' command to remove one or more empty directories.
 * 
 * @param args List of directory names to remove.
 * @return true if the command was handled internally, false otherwise.
 */
bool handleRmdir(const QStringList &args);

/**
 * @brief Handles the 'rm' command to delete files or directories.
 * 
 * Supports the -r, -f, and -rf flags for recursive and forceful deletion.
 * 
 * @param args List of files/directories and optional flags.
 * @return true if the command was handled internally, false otherwise.
 */
bool handleRm(const QStringList &args);

/**
 * @brief Handles the 'mv' command to rename or move files and directories.
 * 
 * Moves files into directories or renames files/folders depending on arguments.
 * 
 * @param args List of source and destination paths.
 * @return true if the command was handled internally, false otherwise.
 */
bool handleMv(const QStringList &args);


signals:
  void processOutputReady(QString output);
  void processErrorReady(QString error);

private:
  QProcess *process;    // Process instance to run commands
  QString command;      // Stores user input command
  QString errorMessage; // Last error message
};

#endif // PROCESS_MANAGER_H
