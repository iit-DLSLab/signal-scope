#include <qapplication.h>
#include "mainwindow.h"
#include "lcmthread.h"

#include "qjson.h"
#include <QMap>
#include <QVariant>

#include <iostream>


int main(int argc, char **argv)
{

    // Input sanity check because roslaunch adds extra args we don't want
    int myargc = argc;
    std::string myarg;
    for(int i=0; i < argc; i++){
        // if the argument starts with __name:= or __log:= we decrease the count
        // NOTE: this only works if the unwanted arguments are the last ones
        myarg = std::string(argv[i]);
        if(myarg.substr(0,strlen("__name:=")).compare("__name:=") == 0){
            myargc--;
        }
        if(myarg.substr(0,strlen("__log:=")).compare("__log:=") == 0){
            myargc--;
        }
    }


  QApplication app(myargc, argv);

  MainWindow window;
  window.show();

  return app.exec();
}
