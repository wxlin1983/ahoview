#include "ahoview.h"
#include "ui_ahoview.h"

//my include
#include <vector>
#include <QString>
#include <QFileDialog>
#include <QMessageBox>
#include <QStringList>
#include <QKeyEvent>
#include <QStatusBar>
#include <QSettings>
#include "zlib.h"
//end my include

ahoview::ahoview(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::ahoview), qimglabel(new QLabel)
{
    ui->setupUi(this);

    setCentralWidget(qimglabel);

    qimglabel->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    qimglabel->setAlignment(Qt::AlignCenter);

    picRescaleMode=0;
    windowSizeMode=0;

    createActions();
    createMenus();

    qstatus=new QLabel(tr("initialized"));
    createStatusbar();

    QSettings ahoset ("aho","ahov",this);
    ahoset.setValue("rescalemode",picRescaleMode);
    ahoset.setValue("windowsizemode",windowSizeMode);
}

ahoview::~ahoview() {
    for (std::vector<picaxiv *>::iterator i = allaxiv.begin(); i != allaxiv.end(); ++i) {delete (*i);}
    delete ui;
}

std::vector<picaxiv *>::iterator ahoview::offset_it(int offset) {
    if (allaxiv.empty()||(allaxiv.size()==1)||(offset==0)) {return axiv_it;}
    std::vector<picaxiv *>::iterator tmp(axiv_it);
    int k=1;
    if (offset<0) {k=-1;}
    for (int i=0;i<offset*k;i++) {
        if (k<0&&tmp==allaxiv.begin()) {tmp=allaxiv.end();}
        tmp=tmp+k;
        if (k>0&&tmp==allaxiv.end()) {tmp=allaxiv.begin();}
    }
    return tmp;
}

int ahoview::changeaxiv(int offset) {
    //doesn't make change if the axiv is not showable
    if ((offset==0)||allaxiv.empty()) {return 1;}
    std::vector<picaxiv *>::iterator tmp=offset_it(offset);
    if (tmp==axiv_it) {return 1;}
    if (!(*tmp)->isvalid()) {return 1;}
    else {
        axiv_it=tmp;
        return 0;
    }
    return 1;
}

int ahoview::closeaxiv(int offset) {
    if (allaxiv.empty()) {return 0;}
    if (allaxiv.size()>1) {
        std::vector<picaxiv *>::iterator tmp=offset_it(offset);
        delete *axiv_it;
        axiv_it=allaxiv.erase(tmp);
        if (axiv_it==allaxiv.end()) {axiv_it=allaxiv.begin();}
        plot();
        return 0;
    } else {
        //size=1.
        delete *axiv_it;
        axiv_it=allaxiv.erase(axiv_it);
        changeStatusbar("no axiv");
        clearplot();
        return 0;
    }
    return 1;
}

void ahoview::closefiledir() {
    closeaxiv(0);
}

void ahoview::openfile() {
    QString fn = QFileDialog::getOpenFileName(this, tr("Open File"), QDir::homePath());
    if (!fn.isEmpty()) {
        picaxiv* picfolderptrtmp=nullptr;
        picfolderptrtmp=new picaxiv(QFileInfo(fn).absoluteFilePath());
        if (picfolderptrtmp!=nullptr) {
            //add file only when it's showable.
            if (picfolderptrtmp->isvalid()) {
                allaxiv.push_back(picfolderptrtmp);
                axiv_it=allaxiv.end()-1;
                plot();
            } else {
                delete picfolderptrtmp;
            }
        }
    }
}

void ahoview::opendir() {
    QString dir = QFileDialog::getExistingDirectory(this,
                                                    tr("Open Folder"), QDir::homePath(),
                                                    QFileDialog::ShowDirsOnly);
    if (!dir.isEmpty()) {
        picaxiv* picfolderptrtmp=nullptr;
        picfolderptrtmp=new picaxiv(QFileInfo(dir).absoluteFilePath());
        if (picfolderptrtmp!=nullptr) {
            //add folder only when it's showable.
            if (picfolderptrtmp->isvalid()) {
                allaxiv.push_back(picfolderptrtmp);
                axiv_it=allaxiv.end()-1;
                plot();
            } else {
                delete picfolderptrtmp;
            }
        }
    }
}

void ahoview::keyPressEvent(QKeyEvent *event) {
    if(event->key() == Qt::Key_Left) {
        if (allaxiv.empty()) {return;}
        std::vector<pic *>::iterator tmp=(*axiv_it)->ptr();
        if (tmp!=(*axiv_it)->mv(-1)) {
            plot();
        }
    }
    else if(event->key() == Qt::Key_Right) {
        if (allaxiv.empty()) {return;}
        std::vector<pic *>::iterator tmp=(*axiv_it)->ptr();
        if (tmp!=(*axiv_it)->mv(1)) {
            plot();
        }
    }
    else if(event->key() == Qt::Key_PageUp) {
        if (allaxiv.empty()) {return;}
        std::vector<pic *>::iterator tmp=(*axiv_it)->ptr();
        if (tmp!=(*axiv_it)->mv(-10)) {
            plot();
        }
    }
    else if(event->key() == Qt::Key_PageDown) {
        if (allaxiv.empty()) {return;}
        std::vector<pic *>::iterator tmp=(*axiv_it)->ptr();
        if (tmp!=(*axiv_it)->mv(10)) {
            plot();
        }
    }
    else if(event->key() == Qt::Key_Up) {
        if (!allaxiv.empty()) {
            if (changeaxiv(1)==0) {
                plot();
            }
        }
    }
    else if(event->key() == Qt::Key_Down) {
        if (!allaxiv.empty()) {
            if (changeaxiv(-1)==0) {
                plot();
            }
        }
    }
    else if(event->key() == Qt::Key_Escape) {
        if (qimglabel->isHidden()) {
            qimglabel->show();
        } else {
            qimglabel->hide();
        }
    } else {
        QWidget::keyPressEvent(event);
    }
}

void ahoview::resizeEvent(QResizeEvent *) {
    plot();
}

void ahoview::plot() {
    if (allaxiv.empty()) {return;}
    //test unload
    std::vector<pic *>::iterator it;
    //it=(*axiv_it)->ptr(-2);
    //if (it!=(*axiv_it)->ptr(0)) {
//        (*axiv_it)->unload(it);
//    }
    //it=(*axiv_it)->ptr(2);
    //if (it!=(*axiv_it)->ptr(0)) {
//        (*axiv_it)->unload(it);
  //  }
    //test end
//    (*axiv_it)->load(1);
//    (*axiv_it)->load(-1);
    (*axiv_it)->scale(0,qimglabel->size(),picRescaleMode);
    if((*(*axiv_it)->ptr())->status!=2) {
        qimglabel->setPixmap((*(*axiv_it)->ptr())->scaled);
        changeStatusbar((*((*axiv_it)->ptr()))->name);
    }
    (*axiv_it)->scale(1,qimglabel->size(),picRescaleMode);
    (*axiv_it)->scale(-1,qimglabel->size(),picRescaleMode);
}

void ahoview::clearplot() {
    qimglabel->clear();
}

void ahoview::createMenus() {
    fileMenu = new QMenu(tr("&File"), this);
    fileMenu->addAction(openfileAct);
    fileMenu->addAction(opendirAct);
    fileMenu->addAction(closeAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    helpMenu = new QMenu(tr("&Help"), this);
    helpMenu->addAction(aboutQtAct);

    menuBar()->addMenu(fileMenu);
    menuBar()->addMenu(helpMenu);
}

void ahoview::createActions() {
    openfileAct = new QAction(tr("Open &file..."), this);
    openfileAct->setShortcut(tr("Ctrl+O"));
    connect(openfileAct, SIGNAL(triggered()), this, SLOT(openfile()));

    opendirAct = new QAction(tr("Open &directory..."), this);
    opendirAct->setShortcut(tr("Ctrl+D"));
    connect(opendirAct, SIGNAL(triggered()), this, SLOT(opendir()));

    closeAct = new QAction(tr("Close file or directory..."), this);
    closeAct->setShortcut(tr("Ctrl+W"));
    connect(closeAct, SIGNAL(triggered()), this, SLOT(closefiledir()));

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcut(tr("Ctrl+Q"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    aboutQtAct = new QAction(tr("About &Qt"), this);
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

void ahoview::createStatusbar() {
    statusBar()->addWidget(qstatus);
}

void ahoview::changeStatusbar(QString newstatus) {
    statusBar()->removeWidget(qstatus);
    delete qstatus;
    qstatus=new QLabel(newstatus);
    statusBar()->addWidget(qstatus);
}
//        QMessageBox msgBox;
//        msgBox.setText("The document has been +1.");
//        msgBox.exec();

//int pic::cpress() {
//    unsigned char buf[1024]={0},strDst[1024]={0};
//    unsigned long srcLen=sizeof(strSrc),bufLen=sizeof(buf),dstLen=sizeof(strDst);

//    compress(buf,&bufLen,strSrc,srcLen);
//    //解压缩
//    uncompress(strDst,&dstLen,buf,bufLen);
//}
