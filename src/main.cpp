#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[]) {
  // create app
  QApplication app(argc, argv);

  // sanity check
  qDebug() << "Hello from Qt application!";
 
  // start event loop
  return app.exec();
}

