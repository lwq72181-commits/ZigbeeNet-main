#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QPath.h"
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    loadConfig();
    //loadIcons();

}

void MainWindow::loadConfig()
{

}

void MainWindow::loadIcons()
{
    QString workdir = QPath::getWorkDir();
    QString imagesDir = QPath::join(workdir, "images");

    QString appPath = QPath::join(imagesDir, "app.png");
    if (QPath::exist(appPath))
    {
        m_appPixmap = QPixmap(appPath);

        QIcon icon(m_appPixmap);
        setWindowIcon(icon);

    }
    else
    {
        m_appPixmap = QPixmap(":/images/app");
    }

}


MainWindow::~MainWindow()
{
    delete ui;
}
