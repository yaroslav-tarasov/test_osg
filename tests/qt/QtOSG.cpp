#include "MainWindow.h"

#include <QApplication>

int main_qtosg( int argc, char** argv )
{
  QApplication application( argc, argv );

  MainWindow mainWindow;
  mainWindow.show();

  return( application.exec() );
}


AUTO_REG(main_qtosg)