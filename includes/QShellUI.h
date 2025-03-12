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

  QString createPrompt(QString username, QString hostname, QString cwd); // method to create shell prompt
  void displayShellPrompt(QString prompt); // method to show prompt

private:
  QPlainTextEdit *editor; // terminal text area 
  QString username = "xande";
  QString hostname = "Zero-Quantity";
  QString cwd = "~";
  QString prompt;

};

#endif // QSHELL_UI_H
