#include "ProcessManager.h"
#include "QShellUI.h"
#include <QDebug>
#include <QDir>
#include <QHostInfo>
#include <QKeyEvent>
#include <QProcessEnvironment>
#include <QTextDocumentFragment>
#include <QTimer>
#include <QApplication>

/**
 * @brief Constructor: Initializes the QShell UI.
 */
QShellUI::QShellUI(QWidget *parent) : QMainWindow(parent) {
  setupUI(); // Setup shell UI

  // create ProcessManager instance
  processManager = new ProcessManager(this);

  // Connect QShellUI command signal to ProcessManager startProcess function
  connect(this, &QShellUI::commandOutputReady, processManager,
          &ProcessManager::startProcess);

  // Connect ProcessManager output signal to QShellUI display function
  connect(processManager, &ProcessManager::processOutputReady, this,
          &QShellUI::displayOutput);
}

/**
 * @brief Destructor: Cleans up resources.
 */
QShellUI::~QShellUI() {}

/**
 * @brief Sets up the terminal UI using a single QTextEdit.
 */
void QShellUI::setupUI() {
  setWindowTitle("QShell");
  resize(800, 600);

  // Create central widget
  QWidget *centralWidget = new QWidget(this);
  mainLayout = new QVBoxLayout(centralWidget);

  // Create a QTextEdit for displaying prompts, input, and output
  terminalArea = new QTextEdit(this);
  terminalArea->setReadOnly(false); // Allow typing
  terminalArea->setStyleSheet(
      "background-color: black; color: green; font-family: monospace;");

  mainLayout->addWidget(terminalArea);
  setCentralWidget(centralWidget);

  // Retrieve system details for prompt
  setUsername();
  setHostname();
  setHomeDIR();
  setCWD();

  // Making sure QShellUI gets key events, without it QTextEdit handles key
  // presses
  terminalArea->installEventFilter(this);

  // Generate and display the initial prompt
  prompt = createPrompt();
  displayShellPrompt();
}

/**
 * @brief Retrieves the system username.
 */
void QShellUI::setUsername() {
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  username = env.value("USER", env.value("USERNAME", "Unknown User"));
}

/**
 * @brief Retrieves the system hostname.
 */
void QShellUI::setHostname() { hostname = QHostInfo::localHostName(); }

/**
 * @brief Retrieves the user's home directory.
 */
void QShellUI::setHomeDIR() {
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  homeDIR = env.value("HOME");
}

/**
 * @brief Sets the current working directory to the home directory.
 */
void QShellUI::setCWD() {
  QDir::setCurrent(homeDIR);
  cwd = "~"; // Display as ~ instead of full path
}

/**
 * @brief Creates the shell prompt string in format: `username@hostname:cwd$ `
 * @return The formatted prompt string.
 */
QString QShellUI::createPrompt() {
  QString rawPrompt =
      QString(
          "<b style='color:yellow;'>%1@%2</b>:<b style='color:blue;'>%3</b>$ ")
          .arg(username, hostname, cwd);

  // Convert to plain text before storing
  return QTextDocumentFragment::fromHtml(rawPrompt).toPlainText();
}

/**
 * @brief Displays a new prompt at the bottom of the terminal.
 */
void QShellUI::displayShellPrompt() {
  terminalArea->moveCursor(QTextCursor::End); // Move cursor to the end

  // Get the last line in the terminal
  QStringList lines = terminalArea->toPlainText().split("\n");
  QString lastLine = lines.isEmpty() ? "" : lines.last().trimmed();

  // If the last line is already a prompt, do NOT add another one
  if (lastLine == prompt.trimmed()) {
    return;
  }

  // prevents from adding a new line to the first time a prompt is displayed
  if (!isFirstPrompt) {
    // add new line before prompt
    terminalArea->insertPlainText("\n");
  }

  terminalArea->insertHtml(prompt);           // Insert the new prompt
  terminalArea->moveCursor(QTextCursor::End); // Ensure cursor is at the end

  // Save position where user input starts (to prevent deleting the prompt)
  promptPosition = terminalArea->textCursor().position();

  // flag first prompt as done
  isFirstPrompt = false;
}

/**
 * @brief Captures user input and prevents backspacing beyond the prompt.
 * @param event The key event triggered by user input.
 */
void QShellUI::keyPressEvent(QKeyEvent *event) {
  // get the cursor
  QTextCursor cursor = terminalArea->textCursor();

  qDebug() << "Cursor Position:" << cursor.position();
  qDebug() << "Prompt Position:" << promptPosition;

  // Prevent moving cursor before the prompt (Left Arrow)
  if (event->key() == Qt::Key_Left) {
    if (cursor.position() <= promptPosition) {
      return; // Ignore left arrow if it's at or before the prompt
    }
  }

  // Prevent moving cursor before the prompt (Up Arrow)
  if (event->key() == Qt::Key_Up) {
    return; // Ignore up arrow to keep cursor within the current command line
  }

  // Prevent Backspace from deleting the prompt
  if (event->key() == Qt::Key_Backspace) {
    if (cursor.position() <= promptPosition) {
      qDebug() << "Backspace blocked to prevent deleting the prompt.";
      return; // Ignore backspace if at prompt position
    }

    qDebug() << "Backspace pressed: Deleting character...";
    cursor.deletePreviousChar(); // Allow deleting user input
    return;
  }

  // Prevent selecting and deleting the prompt
  if (cursor.hasSelection()) {
    if (cursor.selectionStart() < promptPosition ||
        cursor.selectionEnd() < promptPosition) {
      return; // Ignore selection if it includes the prompt
    }
  }

  // Block "Cut" (Ctrl + X) if it includes the prompt
  if (event->matches(QKeySequence::Cut)) {
    return;
  }

  // Block "Paste" (Ctrl + V) if it would overwrite the prompt
  if (event->matches(QKeySequence::Paste)) {
    if (cursor.position() < promptPosition) {
      return;
    }
  }

  // Handle "Enter" key (User submits command)
  if (event->key() == Qt::Key_Return) {
    // Ensure no text is selected
    cursor.clearSelection();

    // Extract the command from QTextEdit using promptPosition
    QString terminalText = terminalArea->toHtml(); // Get full HTML
    terminalText = QTextDocumentFragment::fromHtml(terminalText)
                       .toPlainText(); // Strip formatting

    // Find the last occurrence of the prompt
    int lastPromptIndex = terminalText.lastIndexOf(prompt);
    if (lastPromptIndex == -1) {
      return;
    }

    // Extract the user command correctly
    QString userCommand =
        terminalText.mid(lastPromptIndex + prompt.length()).trimmed();

    // handle exit command
    if (userCommand == "exit") {
      // close shell
      QApplication::quit();
      return;
    }

    // Move the cursor to the end before appending output
    terminalArea->moveCursor(QTextCursor::End);

    // Send command with signal to ProcessManager
    if (!userCommand.isEmpty()) {
      emit commandOutputReady(userCommand); // send command to ProcessManager
    }

    return;
  }

  // Allow typing by manually inserting text into QTextEdit
  if (!event->text().isEmpty()) {
    qDebug() << "Typing detected: " << event->text();
    cursor.insertText(event->text());
    // promptPosition = cursor.position(); // Update prompt position
    return;
  }

  // Default behavior for other keys
  QMainWindow::keyPressEvent(event);
}

/**
 * @brief Filters key press events for the terminal area.
 *
 * This method intercepts key press events targeted at the terminal area
 * (`QTextEdit`). If a key press event occurs in the terminal, it manually calls
 * `keyPressEvent()` to handle user input. This ensures that the terminal
 * behaves as expected.
 *
 * @param object The QObject that received the event.
 * @param event The event being processed.
 * @return `true` if the event was handled, preventing further propagation;
 *         otherwise, forwards it to the default event handler.
 */
bool QShellUI::eventFilter(QObject *object, QEvent *event) {
  if (object == terminalArea && event->type() == QEvent::KeyPress) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
    qDebug() << "[EVENT FILTER] Key pressed: " << keyEvent->key();
    keyPressEvent(keyEvent); // Call keyPressEvent manually

    // If it's Backspace, we mark it as handled to prevent QTextEdit from
    // interfering
    if (keyEvent->key() == Qt::Key_Backspace) {
      return true; // Stop propagation so QTextEdit does not process it
    }

    return true; // Mark event as handled
  }
  return QMainWindow::eventFilter(object, event);
}

void QShellUI::displayOutput(QString output) {
  // ignore empty output
  if (output.trimmed().isEmpty()) {
    return;
  }

  terminalArea->moveCursor(QTextCursor::End); // Ensure cursor is at the end
  terminalArea->insertPlainText("\n" +
                                output.trimmed()); // Append output correctly

  if (!output.endsWith("\n")) {
    // add new line if missing from output
    terminalArea->append("");
  }
  qDebug() << "[DEBUG] Final output before prompt: " << output.right(10);

  // Delay new prompt appears ONLY after the last output
  QTimer::singleShot(15, this, &QShellUI::displayShellPrompt);
}
