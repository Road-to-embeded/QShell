#include "QShellUI.h"
#include <QDir>
#include <QHostInfo>
#include <QProcessEnvironment>
#include <QKeyEvent>

QShellUI::QShellUI(QWidget *parent) : QMainWindow(parent) {
    setupUI();
}

// Destructor
QShellUI::~QShellUI() {}

// UI setup
void QShellUI::setupUI() {
    // Main window properties
    setWindowTitle("QShell");
    resize(800, 600);

    // Central widget
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setStyleSheet("background-color: pink;");
    mainLayout = new QVBoxLayout(centralWidget);

    // Terminal output area (Read-only)
    terminalOutput = new QTextBrowser(this);
    terminalOutput->setStyleSheet("background-color: red; color: green; font-family: monospace;");
    terminalOutput->setReadOnly(true);

    // Input container (Holds prompt + input field)
    inputContainer = new QWidget(this);
    inputLayout = new QHBoxLayout(inputContainer);

    // Prompt label
    promptLabel = new QLabel(this);
    promptLabel->setStyleSheet("background-color: blue; color: green; font-family: monospace;");

    // Input field
    inputField = new QLineEdit(this);
    inputField->setStyleSheet("background-color: gray; color: green; font-family: monospace; border: none;");
    inputField->setFocusPolicy(Qt::StrongFocus); // Ensure cursor starts here

    // Add prompt and input field to the layout
    inputLayout->addWidget(promptLabel);
    inputLayout->addWidget(inputField);
    inputLayout->setStretch(1, 1); // Ensures input field takes most of the space
    inputContainer->setLayout(inputLayout);

    // Add widgets to main layout
    mainLayout->addWidget(inputContainer);
    mainLayout->addWidget(terminalOutput);
    setCentralWidget(centralWidget);
    promptLabel->setFixedHeight(inputField->sizeHint().height()); 

    // Set system details
    setUsername();
    setHostname();
    setHomeDIR();
    setCWD();

    // Generate initial prompt
    prompt = createPrompt();
    displayShellPrompt();

    // Connect inputField to handleUserInput on Enter press
    connect(inputField, &QLineEdit::returnPressed, this, &QShellUI::handleUserInput);
}

// Set username from OS
void QShellUI::setUsername() {
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    username = env.value("USER", env.value("USERNAME", "Unknown User"));
}

// Set hostname from OS
void QShellUI::setHostname() {
    hostname = QHostInfo::localHostName();
}

// Set home directory
void QShellUI::setHomeDIR() {
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    homeDIR = env.value("HOME");
}

// Set current working directory
void QShellUI::setCWD() {
    QDir::setCurrent(homeDIR);
    cwd = "~";
}

// Create prompt
QString QShellUI::createPrompt() {
    return QString("%1@%2:%3$ ").arg(username, hostname, cwd);
}

// Display prompt method
void QShellUI::displayShellPrompt() {
    promptLabel->setText(prompt);
    inputField->clear();
    inputField->setFocus(); // Ensure cursor stays active
}

// Handle user input
void QShellUI::handleUserInput() {
    QString userCommand = inputField->text();
    if (userCommand.isEmpty()) return;

    // Append user input to terminal output
    terminalOutput->append(prompt + userCommand);

    // Simulate command execution (for now, just echoing)
    terminalOutput->append("Executing: " + userCommand);

    // Adjust terminalOutput size to fit content
    resizeTerminalOutput();

    // Move prompt to the bottom
    movePromptToBottom();
}

// Dynamically resize terminalOutput based on content height
void QShellUI::resizeTerminalOutput() {
    terminalOutput->setFixedHeight(terminalOutput->document()->size().height());
}

// Move the prompt + input field to the bottom
void QShellUI::movePromptToBottom() {
    mainLayout->removeWidget(inputContainer);
    mainLayout->addWidget(inputContainer);
    inputField->setFocus(); // Ensure cursor stays in the input field
}

