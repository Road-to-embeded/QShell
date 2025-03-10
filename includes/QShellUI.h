#ifndef QSHELL_UI_H
#define QSHELL_UI_H

#include <QMainWindow>

class QShellUI : public QMainWindow {
  Q_OBJECT // required

public: 
  explicit QShellUI(QWidget *parent = nullptr);

};

#endif // QSHELL_UI_H
