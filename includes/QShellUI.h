#ifndef QSHELLUI_H
#define QSHELLUI_H

#include <QMainWindow>
#include <QTextBrowser>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

class QShellUI : public QMainWindow {
    Q_OBJECT

public:
    explicit QShellUI(QWidget *parent = nullptr);
    ~QShellUI();

private:
    void setupUI();                   // Sets up the UI
    void setUsername();               // Fetches username
    void setHostname();               // Fetches hostname
    void setHomeDIR();                // Fetches home directory
    void setCWD();                    // Sets current working directory
    QString createPrompt();           // Generates the shell prompt
    void displayShellPrompt();        // Displays the prompt in terminal
    void handleUserInput();           // Handles input when Enter is pressed
    void resizeTerminalOutput();      // Dynamically resizes terminal output
    void movePromptToBottom();        // Moves prompt to the bottom

    QTextBrowser *terminalOutput;     // Read-only terminal display
    QLineEdit *inputField;            // User input field
    QLabel *promptLabel;              // Label for prompt
    QWidget *inputContainer;          // Container for prompt + input field
    QHBoxLayout *inputLayout;         // Layout for input section
    QVBoxLayout *mainLayout;          // Main layout

    QString username;
    QString hostname;
    QString homeDIR;
    QString cwd;
    QString prompt;
};

#endif // QSHELLUI_H
