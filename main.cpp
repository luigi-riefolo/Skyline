#include "skyline.h"
#include <QApplication>
#include <GL/glut.h>
#include <vector>

#define SW 700
#define SH 320


int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    Skyline w;
    w.show();

    return a.exec();
}
