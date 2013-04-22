#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <iostream>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <float.h>
#include <fstream>
#include <QDebug>
#include <sstream>
#include <algorithm> // std::make_heap, std::pop_heap, std::push_heap, std::sort_heap
#include <vector>
#include <assert.h>
#include <set>
#include <QProcess>
#include <QProgressBar>
#include "RTree.h"
#include "Split.h"
#include "skyline.h"
#include "ui_skyline.h"


using namespace std;

int dominated(int x, int y, int z);

int xMax = 0, yMax = 0, zMax = 0;
int verbosity = 0;
RTREEMBR *rects;
// Skyline heap
vector<RTREEMBR*> skyline;
// Temporary heap
vector<RTREENODE*> heap;
// Final skyline
vector<RTREEMBR*> finalSkyline;
// Skyline set
set<RTREEMBR*> s;
QString inputFileName, outputFileName;
string attributes[4];
bool fileOpened = false;
bool first = true;
stringstream ss;
bool skGenerated =false;
QProcess process1;
QProcess process2;
int value = 0;
QProgressBar *progress;
bool stopStream = false;


// Initialise RTree
RTREENODE* root = RTreeCreate();

Skyline::Skyline(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Skyline) {
    // Main GUI
    ui->setupUi(this);

    ui->progressBar->setRange(0,100);
    ui->progressBar->setValue(0);
}

Skyline::~Skyline() {
    delete ui;
}

void Skyline::on_quitButton_clicked() {
    qApp->quit();
}

void Skyline::on_actionOpen_triggered() {
    inputFileName = QFileDialog::getOpenFileName(this, tr("Open File"), QString(),
                                                    tr("Text Files (*.txt);;C++ Files (*.cpp *.h)"));
    openFile(inputFileName);
}

void Skyline::printText(QString string) {
    ui->textEdit->append(string);
}


void Skyline::openFile(QString inputFileName) {
    ui->progressBar->setValue(0);

    skyline.clear();
    finalSkyline.clear();
    heap.clear();
    sk.clear();
    int ind = 0;
    // Clear text edit
    ui->textEdit->clear();

    // If file not empty then open file
    if (!inputFileName.isEmpty()) {
        QFile file(inputFileName);
        if (!file.open(QIODevice::ReadOnly| QIODevice::Text)) {
            QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
            return;
        }
        QTextStream in(&file);
        // Read first line and get number of datapoints
        QString line = in.readLine();
        int linesNumber = line.toInt();
        // Read attributes name
        /*line = in.readLine();
        vector<string> v = explodeLine(",", line.toStdString());
        for (ind=0; ind < 4; ind++) {
            attributes[ind] = v[ind];
        }*/

        // Format lineEdit
        ui->lineEdit->setAlignment(Qt::AlignHCenter);
        ui->lineEdit->setText(line);
        while (!in.atEnd()) {
            line = in.readLine();
            // Split line and store datapoint
            vector<string> v = explodeLine(",", line.toStdString());
            // Get coordinates and find maximum values
            int x = atoi(v[0].c_str()); int y = atoi(v[1].c_str()); int z = atoi(v[2].c_str());
            if (x > xMax) xMax = x;
            if (y > yMax) yMax = y;
            if (z > zMax) zMax = z;
            // Get name and store it
            char *str = new char[15];
            strcpy(str,v[3].c_str());
            RTREEMBR *temp = new RTREEMBR { {x,y,z, x,y,z},str };

            RTreeInsertRect(temp,       // the mbr being inserted
                            ind+10,     // i+1 is mbr ID. ID MUST NEVER BE ZERO
                            &root,      // the address of rtree's root since root can change undernieth
                            0);         // always zero which means to add from the root
            delete temp;
            ind++;
        }
        //file.seek(-4);
        file.close();
    }
    // Print loaded filename
    vector<string> name = explodeLine("/", inputFileName.toStdString());
    stringstream ss;
    ss << "File " << name[name.size()-1] << " loaded";
    ui->outputLabel->setText(QString::fromStdString(ss.str()));
    ss.str("");
    fileOpened = true;

}

void Skyline::on_showDataButton_clicked(){
    // Clear text edit
    ui->textEdit->clear();
    if (!fileOpened)
        QMessageBox::critical(this, tr("Error"), tr("No file to load"));

    // If file not empty then open file
    if (!inputFileName.isEmpty()) {
        QFile f(inputFileName);
        if (!f.open(QIODevice::ReadWrite| QIODevice::Text)) {
            QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
            return 1;
        }

        //QTextStream in(&f);
        QByteArray ba = f.readAll();
        QString str(ba);
        ui->textEdit->setText(str);
       /* while (!f.atEnd()) {
            QString line = in.readLine();
            ui->textEdit->setText(line);
        }*/
        f.close();
    }
}


// Check all points in skyline heap, if mbr is dominated return 1 else 0
int Skyline::dominated(int x, int y, int z) {
    RTREEMBR *mbr = new RTREEMBR {{x,y,z,x,y,z}};
    if (verbosity == 1 || verbosity == 2) {
        stringstream s;
        s << "Checking point (" << x << ", " << y << ", " << z << ")...";
        ui->skylineTextBrowser->append(QString::fromStdString(s.str()));
    }
    int r = 0;
    Skyline s;
    int ind=0;
    if (skyline.size() > 0) {
        for (ind=0; ind< (int)skyline.size(); ind++) {
            RTREEMBR *mbr1 = new RTREEMBR {{mbr->bound[0],mbr->bound[1],mbr->bound[2],
                                        mbr->bound[3],mbr->bound[4],mbr->bound[5]}};
    RTREEMBR *sklRect = new RTREEMBR {{skyline[ind]->bound[0],skyline[ind]->bound[1],skyline[ind]->bound[2],xMax,yMax,zMax}};
            // Is mbr1 contained (dominated) by sklRect???
            r = RTreeContain(mbr1,sklRect);
            if (r == 1) {
                if (verbosity == 1 || verbosity == 2) {
                    stringstream s;
                    s << "\tDominated by point (" << (int)sklRect->bound[0] << ", " << (int)sklRect->bound[1] << ", " << (int)sklRect->bound[2] << ")";
                    ui->skylineTextBrowser->append(QString::fromStdString(s.str()));
                }
                    return r;
            }
            delete mbr1;
            delete sklRect;
        }
        if ((verbosity==1 && r == 0) || (verbosity==2  && r == 0))
            ui->skylineTextBrowser->append("\tNot dominated by any point");
    }

    return r;
}


bool Skyline::skylineGenerated(void) {
    return skGenerated;
}

int Skyline::on_skylineButton_clicked() {
    ui->skylineTextBrowser->clear();
    if (!fileOpened) {
        QMessageBox::critical(this, tr("Error"), tr("No file to load"));
        return 1;
    }
    updateProgressBar();
    int ind,res;
    // Create heap
    make_heap(skyline.begin(),skyline.end());
    make_heap (heap.begin(),heap.end());
    updateProgressBar();

    ui->skylineTextBrowser->append("Calculating skyline...");
    updateProgressBar();
    // Insert all entries of the root R in the heap
    for (ind=0; ind< root->count; ind++) {
        heap.push_back(root->branch[ind].child);
        push_heap(heap.begin(),heap.end());
    }
    updateProgressBar();
    sort_heap(heap.begin(),heap.end());
    updateProgressBar();
    // While heap not empty
    while (!heap.empty()) {cout << "Sk size:" << skyline.size() << endl;
        // Remove top entry e
        RTREENODE *e = heap[0];
        RTREEMBR *mbr = &(RTreeNodeCover( e ));
        heap.erase(heap.begin());
        // If e is dominated by some point in S discard e
        res = dominated(mbr->bound[0],mbr->bound[1],mbr->bound[2]);
        if (res == 1) {
            continue;
        }
        else { 					// e is not dominated
            // if e is an intermediate entry
            if ((e->count > 0) & (e->level > 0)) {                      // if is intermediate (count>0) go on
                if (verbosity==1) printText("Intermediate node"); 	// if is leaf and contains data, then
                // for each child ei of e							// for each mbr contained execute
                for (ind=0; ind < e->count; ind++) {
                    int rs = dominated(e->branch[ind].mbr.bound[0],
                                         e->branch[ind].mbr.bound[1],
                                         e->branch[ind].mbr.bound[2]);
                    // if ei is not dominated by some point in S
                    if (rs == 0) {
                        // insert ei into heap
                        heap.push_back(e->branch[ind].child);
                        push_heap(heap.begin(),heap.end());
                        sort_heap(heap.begin(),heap.end());
                    }
                }
            }
            else if ((e->count > 0) & (e->level == 0)) {
                // for each child ei of e
                for (ind=0; ind < e->count; ind++) {
                    //int rs = isDominated(&(e->branch[ind].mbr));
                    int rs = dominated(e->branch[ind].mbr.bound[0],
                                       e->branch[ind].mbr.bound[1],
                                       e->branch[ind].mbr.bound[2]);
                    // if ei is not dominated by some point in S
                    if (rs == 0) {
                        // insert ei into heap
                        skyline.push_back(&e->branch[ind].mbr);
                        push_heap(skyline.begin(),skyline.end());
                        if (verbosity==1 || verbosity ==2) {
                            if (skyline.size() == 1)
                                    ui->skylineTextBrowser->append("First point in skyline:");
                                else
                                    ui->skylineTextBrowser->append("1New point in skyline:");
                                string tmp;
                                tmp = RTreeGetRectName(&e->branch[ind].mbr,0);
                                ui->skylineTextBrowser->append(QString::fromStdString(tmp));
                        }
                    }
                }
            }
            // else e is a data point
            else {
                // insert e into skyline
                RTREEMBR *s = &(RTreeNodeCover( e ));
                skyline.push_back(s);
                push_heap(skyline.begin(),skyline.end());
                if (verbosity==1 || verbosity == 2) {
                    if (skyline.size() == 1)
                        ui->skylineTextBrowser->append("First point in skyline:");
                    else
                        ui->skylineTextBrowser->append("2New point in skyline:");
                    string tmp;
                    tmp = RTreeGetRectName(s,0);
                    ui->skylineTextBrowser->append(QString::fromStdString(tmp));

                }
                delete s;
            }
        }
    }
    ui->progressBar->setValue(85);
    // Check for points in skyline that are dominated
    updateSkyline();
    removeDuplicates();
    printFinalSkyline();
    sort_heap(sk.begin(),sk.end());
    writeOutputFile();
    skGenerated = true;

    heap.clear();
    //RTreeDestroy(root);

    return 0;
}

void Skyline::printFinalSkyline(void) {
    sort_heap(finalSkyline.begin(),finalSkyline.end());
    completeProgressBar();
    string temp;
    int ind;
    // Print skyline
    ui->skylineTextBrowser->append("\nSkyline content: ");
    cout << "Size " << finalSkyline.size() << endl;
    for (ind = 0; ind <(int)finalSkyline.size(); ind++) {
        Point *tmp = new Point{finalSkyline[ind]->bound[0], finalSkyline[ind]->bound[1],finalSkyline[ind]->bound[2], finalSkyline[ind]->data};
        temp = RTreeGetRectName(finalSkyline[ind],0);
        ui->skylineTextBrowser->append(QString::fromStdString(temp));
        sk.push_back(tmp);
        push_heap(sk.begin(), sk.end());
    }
}


void Skyline::writeOutputFile(void) {
    if( remove( "temp.txt" ) != 0 )
        perror( "Error deleting file" );
    else
        puts( "File successfully deleted" );

    ofstream outFile;
    outFile.open("temp.txt");
    int ind;
    for (ind = 0; ind < (int)sk.size();ind++) {
        outFile << sk[ind]->x << "," << sk[ind]->y << "," << sk[ind]->z << "," << sk[ind]->name << "\n";
    }
    outFile.close();

}


void Skyline::updateSkyline(void) {
    ui->skylineTextBrowser->append("\nModifying skyline...");
    make_heap(finalSkyline.begin(),finalSkyline.end());
    int ind;
    for (ind = 0; ind < (int)skyline.size(); ind++) {
        int rs = dominated(skyline[ind]->bound[0],skyline[ind]->bound[1],skyline[ind]->bound[2]);
        if (rs == 0) {
            finalSkyline.push_back(skyline[ind]);
            push_heap(finalSkyline.begin(),finalSkyline.end());
        }
    }
}


void Skyline::on_spinBox_valueChanged(int arg1) {
    verbosity = arg1;
}

void Skyline::on_pushButton_clicked() {
    if (!process1.atEnd()) {
        process1.terminate();
        process1.close();
    }
    if(!skGenerated) {
        QMessageBox::critical(this, tr("Error"), tr("Skyline not generated"));
        return 1;
    }
    process1.start("./Visualiser");
}

QPushButton *stopButton;
void Skyline::on_stopButton_clicked() {
    cout << "stop" << endl;
    stopStream = true;
    //ui->horizontalLayout->removeWidget(stopButton);
}

void Skyline::updateProgressBar() {
    value++;
    ui->progressBar->setValue(value);
}

void Skyline::completeProgressBar(){
    value = ui->progressBar->value();
    ui->progressBar->setValue((100-value)+value);
}


int Skyline::streamDominated(int x, int y, int z) {
    int ind=0;

    if (verbosity == 1 || verbosity == 2) {
        stringstream s;
        s << "Checking point (" << x << ", " << y << ", " << z << ")...";
        ui->skylineTextBrowser->append(QString::fromStdString(s.str()));
    }
    int r = 0;

    if (sk.size() > 0) {
        for (ind=0; ind< (int)sk.size(); ind++) {
            RTREEMBR *mbr = new RTREEMBR {{x,y,z,xMax,yMax,zMax}};
            RTREEMBR *sklRect = new RTREEMBR {{sk[ind]->x,sk[ind]->y,sk[ind]->z,xMax,yMax,zMax}};
            // Is mbr1 contained (dominated) by sklRect???
            r = RTreeContain(mbr,sklRect);
            if (r == 1) {
                if (verbosity == 1 || verbosity == 2) {
                    stringstream s;
                    s << "\tDominated by point (" << (int)sklRect->bound[0] << ", " << (int)sklRect->bound[1] << ", " << (int)sklRect->bound[2] << ")";
                    ui->skylineTextBrowser->append(QString::fromStdString(s.str()));
                }
                    return r;
            }
            delete mbr1;
            delete sklRect;
        }
        if ((verbosity==1 && r == 0) || (verbosity==2  && r == 0))
            ui->skylineTextBrowser->append("\tNot dominated by any point");
    }

    return r;
}
dfsfd

void Skyline::on_actionOpen_URL_triggered() {
    int ind = 0;
    stopButton = new QPushButton("Stop", this);
    ui->horizontalLayout->addWidget(stopButton);
    connect(stopButton, SIGNAL(clicked()), this, SLOT(on_stopButton_clicked()));

    //process1.start("./listener.py");

    QString inputFileName = "/home/sonda/Desktop/SkylineApp/stream.txt";

    ui->skylineTextBrowser->append("\nReading from localhost(127.0.0.1)...\n");

    // If file not empty then open file
    if (!inputFileName.isEmpty()) {
        QFile file(inputFileName);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
            return;
        }
        QTextStream in(&file);
        QString line = in.readLine();
        while (!in.atEnd() & !stopStream) {
            line = in.readLine();
            // Split line and store datapoint
            vector<string> v = explodeLine(",", line.toStdString());
            // Get coordinates and find maximum values
            int x = atoi(v[0].c_str()); int y = atoi(v[1].c_str()); int z = atoi(v[2].c_str());
            if (x > xMax) xMax = x;
            if (y > yMax) yMax = y;
            if (z > zMax) zMax = z;
            // Get name and store it
            char *str = new char[15];
            strcpy(str,v[3].c_str());
            RTREEMBR *temp = new RTREEMBR { {x,y,z, x,y,z},str };
            ss << "Analysing point " << str << " (" << x << ", " << y << ", " << z << ")...";
            ui->skylineTextBrowser->append(QString::fromStdString(ss.str()));
            ss.str("");
            int r = streamDominated(x,y,z);
            if (r == 0) {
                cout << "Not dom" << endl;
                Point *tmp = new Point{x,y,z,v[3]};
                sk.push_back(tmp);
                push_heap(sk.begin(),sk.end());
            }
            ind++;
        }

    }
    file.close();
    updateSkyline();
    removeDuplicates();
    printFinalOutput();
    skGenerated = true;
}


void Skyline::removeDuplicates(void) {

    unsigned size = finalSkyline.size();
    for( unsigned i = 0; i < size; ++i )
        s.insert( finalSkyline[i] );
    finalSkyline.assign( s.begin(), s.end() );
}

void Skyline::printFinalOutput(void) {
    int ind = 0;
    if( remove( "temp.txt" ) != 0 )
        perror( "Error deleting file" );
    else
        puts( "File successfully deleted" );

    cout << sk.size() << endl;
    ofstream outFile;
    outFile.open("temp.txt");
    ui->skylineTextBrowser->append(QString::fromStdString("Final content:"));
    for (ind = 0; ind < (int)sk.size();ind++) {
        ss << sk[ind]->x << "," << sk[ind]->y << "," << sk[ind]->z << "," << sk[ind]->name;
        outFile << ss.str() << endl;
        ui->skylineTextBrowser->append(QString::fromStdString(ss.str()));
        ss.str("");
    }
    outFile.close();

    skyline.clear();
    finalSkyline.clear();
    heap.clear();
    sk.clear();
}
