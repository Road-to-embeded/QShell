#include "ProcessManager.h"
#include "QShellUI.h"
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHostInfo>
#include <QKeyEvent>
#include <QProcessEnvironment>
#include <QScrollBar>
#include <QTextDocumentFragment>
#include <QTimer>

/**
 * @brief Constructor: Initializes the QShell UI.
 */
QShellUI::QShellUI(QWidget *parent) : QMainWindow(parent) {
  setupUI();        // Setup shell UI
  loadStyleSheet(); // load default styles

  // create ProcessManager instance
  processManager = new ProcessManager(this);

  // Connect QShellUI command signal to ProcessManager startProcess function
  connect(this, &QShellUI::commandOutputReady, processManager,
          &ProcessManager::startProcess);

  // Connect ProcessManager output signal to QShellUI display function
  connect(processManager, &ProcessManager::processOutputReady, this,
          static_cast<void (QShellUI::*)(QString)>(&QShellUI::displayOutput));

  // Connect ProcessManager output error 
  connect(processManager, &ProcessManager::processErrorReady, this, &QShellUI::displayError);
}

/**
 * @brief Destructor: Cleans up resources.
 */
QShellUI::~QShellUI() {}

/*
 * @brief Load stylesheet
 */
void QShellUI::loadStyleSheet() {
  // find executable path
  QString qAppDIR = QCoreApplication::applicationDirPath();

  // adjusting path to stylesheet
  QString qssPath = QDir(qAppDIR).filePath("../resources/styles.qss");

  // load resources
  QFile stylesFile(qssPath);

  // open file on readonly mode
  if (stylesFile.open(QFile::ReadOnly)) {
    // Convert QByteArray to QString
    QString styleSheet = QLatin1String(stylesFile.readAll());

    // apply styles globally using qApp(pointer to )
    qApp->setStyleSheet(styleSheet);
    stylesFile.close();
  } else {
    qDebug() << "Error loading QSS file.";
  }
}

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

  terminalArea->setCursorWidth(8);
  terminalArea->setReadOnly(false); // Allow typing

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
  QString rawPrompt = QString("%1@%2:%3$").arg(username, hostname, cwd);
  // Convert to plain text before storing
  return QTextDocumentFragment::fromHtml(rawPrompt).toPlainText();
}

/**
 * @brief Displays a new prompt at the bottom of the terminal.
 */
void QShellUI::displayShellPrompt() {
  terminalArea->moveCursor(QTextCursor::End); // Move cursor to the end

  // refresh current working directory (this handles the cd command prompt
  // update)
  cwd = QDir::currentPath();

  // replace home DIR with '~'
  QString homePath = QDir::homePath();
  if (cwd.startsWith(homePath)) {
    cwd.replace(0, homePath.length(), "~");
  }

  // create prompt with updated path
  prompt = createPrompt();

  // Get the last line in the terminal
  QStringList lines = terminalArea->toPlainText().split("\n");
  QString lastLine = lines.isEmpty() ? "" : lines.last().trimmed();

  // prevents double prompts from stacking up when the user presses Enter multiple times
  if ((lastLine == prompt.trimmed()) && !isFirstPrompt) {
    return;
  }

  // prevents from adding a new line to the first time a prompt is displayed
  if (!isFirstPrompt) {
    // add new line before prompt
    terminalArea->insertPlainText("\n");
  }

  // apply class property (QTextEdit area styles)
  terminalArea->setObjectName("defaultTerminal");

  // apply default style
  QString styledPrompt =
      QString("<span style='font: monospace; font-weight: bold;'>"
              "<span style='font-weight: bold; color: "
              "#9BDB0F;'>%1@%2</span>:"
              "<span style='color: #11E3DF;'>%3</span>$ "
              "</span>")
          .arg(username, hostname, cwd);

  terminalArea->insertHtml(styledPrompt);     // Insert the new prompt
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
      return; // Ignore backspace if at prompt position
    }

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

  // Handle clear screen
  if (event->key() == Qt::Key_L && event->modifiers() & Qt::ControlModifier) {
    // scroll up effect inserting new lines to clear screen
    clearScreen();

    return;
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

    // Store last command
    lastCommand = userCommand;


    // trigger prompt on empty command
    if (userCommand.isEmpty()) {
      displayOutput("", "");
      return;
    }

    // handle exit command
    if (userCommand == "exit") {
      // close shell
      QApplication::quit();
      return;
    }

    // handle DIR changes
    if (userCommand.startsWith("cd")) {
      // placeholders
      QString newDIR;

      // get command arguments
      QStringList args = userCommand.split(" ", Qt::SkipEmptyParts);

      if (args.size() == 1) {
        // go home directory
        newDIR = QDir::homePath();
      } else {
        // grab new path
        newDIR = args[1];
      }

      // validate path existance
      if (QDir(newDIR).exists()) {
        // change working DIR
        QDir::setCurrent(newDIR);

        // update shells current working directory
        cwd = QDir::currentPath();

        // replace home directory with "~"
        QString homePath = QDir::homePath();
        if (cwd.startsWith(homePath)) {
          cwd.replace(0, homePath.length(), "~");
        }

        // generate prompt with new path
        prompt = createPrompt();
      } else {
        displayOutput("cd: no such file or directory: " + newDIR);
      }

      // display new prompt with new DIR
      displayShellPrompt();

      // stop further processing of the command (No need to send command to
      // child Process)
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

/*
 * @brief Slot handler
 */
void QShellUI::displayOutput(QString output) {
  displayOutput(output, lastCommand);
}

/*
 * @brief Display output override to handle user input
 *
 * @params command output and command itself
 * */
void QShellUI::displayOutput(QString output, QString command) {
  // If user just pressed Enter (empty command), just show a new prompt
  if (command.trimmed().isEmpty()) { 
    // position cursor
    QTextCursor cursor = terminalArea->textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.insertBlock(); // inserts a new line

    terminalArea->setTextCursor(cursor);
    
    isFirstPrompt = true; // allow new line without double spacing
    displayShellPrompt();
    return;
}
    // Ignore empty output
  if (output.trimmed().isEmpty()) {

    displayShellPrompt();
    return;
  }

  terminalArea->moveCursor(QTextCursor::End);      // Move cursor to the end
  QTextCursor cursor = terminalArea->textCursor(); // Insert output as new block
  cursor.insertBlock();                            // New line before output

  // Apply formatting using QTextCharFormat
  QTextCharFormat format;
  format.setForeground(QColor("#11E3DF")); // Cyan color for output
  cursor.setCharFormat(format);

  // validate for 0 args
  // validate for no dir/unknown
  // ls on empty should call prompt 
  
  // Handle 'ls' command formatting
  if (command.trimmed().startsWith("ls")) { 
    // format and clorize directories
    QString styledLSOutput = styleOutput(output);
    cursor.insertHtml(styledLSOutput);
  } else {
    // Insert output
    cursor.insertText(output.trimmed());
  }

  terminalArea->moveCursor(QTextCursor::End); // Move cursor to end

  // Delay new prompt after last output
  QTimer::singleShot(15, this, &QShellUI::displayShellPrompt);
}

void QShellUI::clearScreen() {
  terminalArea->clear(); // Clears everything

  // Reset cursor position to absolute start
  terminalArea->moveCursor(QTextCursor::Start);

  // Reset first prompt flag to ensure it doesn't add a new line
  isFirstPrompt = true;

  // Reset scrollbar position to the top
  QScrollBar *scrollBar = terminalArea->verticalScrollBar();
  if (scrollBar) {
    scrollBar->setValue(scrollBar->minimum());
  }

  // Force a full UI refresh to reflect changes
  terminalArea->update();

  // Add new prompt at the top
  displayShellPrompt();
}

// Style output for ls commmand
QString QShellUI::styleOutput(QString output) {
  // get each output entry
  QStringList entries = output.split("\n", Qt::SkipEmptyParts);
  QStringList styledOutput;

  // apply style to each entry
  for (const QString entry : entries) {
    // place holder
    QString styledEntry;

    // file type
    QFileInfo fileType(entry.trimmed());

    // apply style
    if (fileType.isDir()) {
      styledEntry = "<span style='color:steelblue'>" + entry + "</span>";
    }

    else {

      styledEntry = entry;
    }

    // build styled output
    styledOutput.append(styledEntry);
  }

  return styledOutput.join("<br>");
}

// display error implementation
void QShellUI::displayError(QString error) {
    terminalArea->moveCursor(QTextCursor::End);
    QTextCursor cursor = terminalArea->textCursor();

    // Only insert block if last character isn't already a newline
    QString lastChar = terminalArea->toPlainText().right(1);
    if (lastChar != "\n") {
        cursor.insertBlock();  // Only insert block when needed
    }

    QTextCharFormat errorFormat;
    errorFormat.setForeground(QColor("#FF5555")); // Light red
    cursor.setCharFormat(errorFormat);
    cursor.insertText(error.trimmed());

    QTimer::singleShot(15, this, &QShellUI::displayShellPrompt);
}

