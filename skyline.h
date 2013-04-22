#ifndef SKYLINE_H
#define SKYLINE_H

#include <QMainWindow>
#include <vector>
#include <string>
#include <GL/glut.h>
#include <QProgressBar>

typedef struct {
  GLfloat x;
  GLfloat y;
  GLfloat z;
  std::string name;
} Point;

static std::vector<Point*> sk;

namespace Ui {
    class Skyline;
}

class Skyline : public QMainWindow
{
    Q_OBJECT

public:
    explicit Skyline(QWidget *parent = 0);
    ~Skyline();
    void openFile(QString fileName);
    void printText(QString string);
    void printThis(void);
    int dominated(int x, int y, int z);
    bool skylineGenerated(void);
    void updateProgressBar(void);
    void updateSkyline(void);
    void printFinalSkyline(void);
    int streamDominated(int x, int y, int z);
    void removeDuplicates(void);
    void printFinalOutput(void);

private slots:
    void on_quitButton_clicked();
    void on_actionOpen_triggered();
    void on_showDataButton_clicked();
    int on_skylineButton_clicked();
    void on_spinBox_valueChanged(int arg1);
    void on_pushButton_clicked();
    void writeOutputFile(void);
    void completeProgressBar(void);
    void on_actionOpen_URL_triggered();
    void on_stopButton_clicked();

private:
    Ui::Skyline *ui;
};

#endif // SKYLINE_H
