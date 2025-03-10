#include <QApplication>
#include <QDebug>
#include <QShellUI.h>

int main(int argc, char *argv[]) {
  // create app
  QApplication app(argc, argv);

  // create main window
  QShellUI window;

  // render window
  window.show();
 
  // start event loop
  return app.exec();
}

