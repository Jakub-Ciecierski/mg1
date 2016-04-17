#include "editor_window.h"
#include "ui_mainwindow.h"
#include "widgets/objects_list/objects_tree_factory.h"

#include <widgets/objects_list/context_menus/objects_cmenu_factory.h>
#include <widgets/scene_list/context_menus/scene_cmenu_factory.h>
#include <iostream>

#include <QApplication>
#include <QMessageBox>
#include <QDebug>
#include <QInputDialog>

using namespace std;

EditorWindow::EditorWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    ObjectsTreeFactory objTreeFactory(ui->objectsListTree);
    objTreeFactory.create();

    setupProperties();
    setupContextMenus();
    setupBinding();
    setupSplitters();

}

EditorWindow::~EditorWindow()
{
    delete ui;

    delete objectListContextMenu;
}

//-------------------------//
//  PUBLIC METHODS
//-------------------------//

EditorWindow& EditorWindow::getInstance(){
    static EditorWindow instance;

    return instance;
}

Ui::MainWindow* EditorWindow::getUI(){
    return this->ui;
}

ContextMenu& EditorWindow::getObjectsListContextMenu(){
    return *objectListContextMenu;
}

bool EditorWindow::showQuestionBox(string title, string text){
    QMessageBox::StandardButton reply;
      reply = QMessageBox::question(this, QString::fromStdString(title),
                                    QString::fromStdString(text),
                                    QMessageBox::Yes|QMessageBox::No);
      if (reply == QMessageBox::Yes) {
        return true;

      }
      return false;
}

void EditorWindow::showInfoBox(string title, string text){
    //QMessageBox::StandardButton reply;
    QMessageBox::question(this, QString::fromStdString(title),
                          QString::fromStdString(text),
                          QMessageBox::Yes|QMessageBox::No);
}

string EditorWindow::showInputBox(string title, string text){
    QInputDialog inputDialog;
    inputDialog.setOptions(QInputDialog::NoButtons);

    bool ok;
    QString reply =  inputDialog.getText(this ,QString::fromStdString(title),
                                        QString::fromStdString(text),
                                        QLineEdit::Normal,
                                        QDir::home().dirName(), &ok);
    if (ok && !reply.isEmpty())
    {
        return reply.toStdString();
    }
    return "";
}

//-------------------------//
//  PRIVATE METHODS
//-------------------------//

void EditorWindow::setupProperties(){
    QLineEdit* x = ui->positionXInput;
    QLineEdit* y = ui->positionYInput;
    QLineEdit* z = ui->positionZInput;

    QDoubleValidator* validator =
            new QDoubleValidator(-1000.0, 1000.0, 2, NULL);

    x->setValidator(validator);
    y->setValidator(validator);
    z->setValidator(validator);

    x->setText("0.0");
    y->setText("0.0");
    z->setText("0.0");
}

void EditorWindow::setupContextMenus(){
    ObjectsCMenuFactory objCMFactory;

    objectListContextMenu = objCMFactory.create();
/*
    connect(ui->glRendererWidget, SIGNAL(customContextMenuRequested(const QPoint&)),
        ui->sceneTree, SLOT(ShowContextMenu(const QPoint&)));
        */
}

void EditorWindow::setupBinding(){

}

void EditorWindow::setupSplitters(){
    ui->objectsTreeToRenderSplitter->setStretchFactor(0,2);
    ui->objectsTreeToRenderSplitter->setStretchFactor(1,20);

    ui->renderToPropertiesSplitter->setStretchFactor(0,10);
    ui->renderToPropertiesSplitter->setStretchFactor(1,2);

    ui->sceneProperitesSplitter->setStretchFactor(0,2);
    ui->sceneProperitesSplitter->setStretchFactor(1,1);

    ui->renderToPropertiesSplitter->
            setStyleSheet("QSplitter::handle{background: orange;}");
    ui->objectsTreeToRenderSplitter->
            setStyleSheet("QSplitter::handle{background: orange;}");

}
