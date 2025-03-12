#ifndef QSHELL_UI_H
#define QSHELL_UI_H

#include <QMainWindow>
#include <QPlainTextEdit>
#include <QString>

class QShellUI : public QMainWindow {
  Q_OBJECT // required

public: 
  explicit QShellUI(QWidget *parent = nullptr);
  ~QShellUI();

  void setUsername(); // method to set username from OS
  void setHostname(); // method sets hostname from OS
  QString createPrompt(QString username, QString hostname, QString cwd); // method to create shell prompt
  void displayShellPrompt(QString prompt); // method to show prompt

private:
  QPlainTextEdit *editor; // terminal text area 
  QString username;
  QString hostname;
  QString cwd = "~";
  QString prompt;

};

#endif // QSHELL_UI_H
