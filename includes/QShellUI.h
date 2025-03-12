#ifndef QSHELL_UI_H
#define QSHELL_UI_H

#include <QMainWindow>
#include <QPlainTextEdit>

class QShellUI : public QMainWindow {
  Q_OBJECT // required

public: 
  explicit QShellUI(QWidget *parent = nullptr);
  ~QShellUI();

  void displayShellPrompt(); // method to show prompt

private:
  QPlainTextEdit *editor; // terminal text area 

};

#endif // QSHELL_UI_H
