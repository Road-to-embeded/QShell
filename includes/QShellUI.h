#ifndef QSHELLUI_H
#define QSHELLUI_H

#include "ProcessManager.h"
#include <QMainWindow>
#include <QString>
#include <QTextEdit>
#include <QVBoxLayout>
/**
 * @brief The QShellUI class creates a simple terminal emulator.
 *
 * - Uses a single QTextEdit to handle both input and output.
 * - Prevents backspacing past the prompt.
 * - Displays the command after Enter is pressed (without execution).
 */
class QShellUI : public QMainWindow {
  Q_OBJECT

public:
  /**
   * @brief Constructs the QShellUI terminal.
   * @param parent The parent widget.
   */
  explicit QShellUI(QWidget *parent = nullptr);

  /**
   * @brief Destructor.
   */
  ~QShellUI();

  /**
   * @brief Intercept key events before QTextEdit handles them
   */
  bool eventFilter(QObject *object, QEvent *event) override;

  /*
   *@brief Loads stylesheet file into QShellUI
   */
  void loadStyleSheet();

signals:
  /*
   * @brief Send command output ready signal to ProcessManager
   *
   */
  void commandOutputReady(QString command);

private slots:
  /*
   * @brief Receives output from ProcessManager
   *
   */
  void displayOutput(QString output);

protected:
  /**
   * @brief Handles keyboard input to:
   * - Prevent deleting past the prompt.
   * - Detect when Enter is pressed to echo the command.
   * @param event The key event triggered by user input.
   */
  void keyPressEvent(QKeyEvent *event) override;

private:
  /**
   * @brief Sets up the UI, creating a QTextEdit as the terminal.
   */
  void setupUI();

  /**
   * @brief Retrieves the system username.
   */
  void setUsername();

  /**
   * @brief Retrieves the system hostname.
   */
  void setHostname();

  /**
   * @brief Retrieves the user's home directory.
   */
  void setHomeDIR();

  /**
   * @brief Sets the current working directory to the home directory.
   */
  void setCWD();

  /**
   * @brief Generates the shell prompt in the format: `username@hostname:cwd$`
   * @return The formatted prompt string.
   */
  QString createPrompt();

  /**
   * @brief Displays a new shell prompt at the bottom of the terminal.
   */
  void displayShellPrompt();

  /**
   * @brief Handles user input and echoes it to the terminal.
   */
  void handleUserInput();

  /*
   * @brief Clear screen by pushing output upward
   */
  void clearScreen(); 

  QTextEdit *terminalArea; // Terminal display area (both input & output).
  QVBoxLayout *mainLayout; // Layout manager for UI elements.
  ProcessManager *processManager; // ShellUI create a ProcessManager
  QString username;               // Stores the current system username.
  QString hostname;               // Stores the system hostname.
  QString homeDIR;                // Stores the home directory.
  QString cwd;                    // Stores the current working directory.
  QString prompt;                 // Stores the generated prompt.
  int promptPosition;        // Tracks where user input starts to prevent prompt
                             // deletion.
  bool isFirstPrompt = true; // Frist prompt flag to track displayShellPrompt
                             // very first time been called.
};

#endif // QSHELLUI_H
