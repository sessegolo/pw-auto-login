#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QProcess>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_model(new CharModel)
    , m_newCharData()
    , m_pidList()
{
    ui->setupUi(this);
    setWindowFlags(Qt::Widget);

    ui->charsTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->charsTableView->verticalHeader()->hide();
    ui->charsTableView->setModel(m_model);

    setupConnections();
    readSettings();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupConnections()
{
    connect(ui->elementClientButton, &QPushButton::clicked, this, &MainWindow::openFileDialog);
    connect(ui->accountLineEdit, &QLineEdit::editingFinished, this, &MainWindow::updateCharAccount);
    connect(ui->passwordLineEdit, &QLineEdit::editingFinished, this, &MainWindow::updateCharPassword);
    connect(ui->charLineEdit, &QLineEdit::editingFinished, this, &MainWindow::updateCharName);
    connect(ui->addCharButton, &QPushButton::clicked, this, &MainWindow::addChar);
    connect(ui->clearFormButton, &QPushButton::clicked, this, &MainWindow::clearForm);
    connect(ui->logCharButton, &QPushButton::clicked, this, &MainWindow::logSelectedChar);
    connect(ui->deleteCharButton, &QPushButton::clicked, this, &MainWindow::deleteChar);
}

void MainWindow::readSettings()
{
    QSettings settings;

    ui->unfreezeCheckbox->setChecked(settings.value("unfreeze", false).toBool());
    ui->elementClientLineEdit->setText(settings.value("elementClientPath", "").toString());

    int charCount = settings.value("charCount", 0).toInt();
    for(int i = 0; i < charCount; ++i)
    {
        QStringList charInfo = settings.value(QString("char%1").arg(i), QStringList()).toStringList();
        CharData charData;
        charData.setAccount(charInfo.at(0));
        charData.setPassword(charInfo.at(1));
        charData.setCharName(charInfo.at(2));
        m_model->pushCharData(charData);
    }
}

void MainWindow::writeSettings()
{
    QSettings settings;

    settings.setValue("unfreeze", ui->unfreezeCheckbox->isChecked());
    settings.setValue("elementClientPath", ui->elementClientLineEdit->text());

    int charCount = m_model->rowCount();
    settings.setValue("charCount", charCount);
    for(int i = 0; i < charCount; ++i)
    {
        settings.setValue(QString("char%1").arg(i), m_model->getCharData(i).toList());
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}

// ------------ SLOTS ------------
void MainWindow::openFileDialog()
{
    QString elementClientPath = QFileDialog::getOpenFileName(this,
                                                             tr("Buscar ElementClient"), "C:/",
                                                             tr("ElementClient (ElementClient.exe)"));
    ui->elementClientLineEdit->setText(elementClientPath);
}

void MainWindow::updateCharAccount()
{
    m_newCharData.setAccount(ui->accountLineEdit->text());
}

void MainWindow::updateCharPassword()
{
    m_newCharData.setPassword(ui->passwordLineEdit->text());
}

void MainWindow::updateCharName()
{
    m_newCharData.setCharName(ui->charLineEdit->text());
}

void MainWindow::addChar()
{
    if(m_newCharData.getAccount() == "" || m_newCharData.getPassword() == "")
    {
        ui->statusbar->showMessage("ERRO: Conta/senha vazia", 2000);
        return;
    }
    else if(m_newCharData.getCharName() == "")
    {
        ui->statusbar->showMessage("AVISO: Nick vazio (opcional)", 2000);
    }

    m_model->pushCharData(m_newCharData);
    m_newCharData = CharData();
    ui->accountLineEdit->clear();
    ui->passwordLineEdit->clear();
    ui->charLineEdit->clear();

    ui->statusbar->showMessage("Conta adicionada!", 2000);
}

void MainWindow::clearForm()
{
    m_newCharData = CharData();
    ui->accountLineEdit->clear();
    ui->passwordLineEdit->clear();
    ui->charLineEdit->clear();
}

void MainWindow::logSelectedChar()
{
    if(ui->elementClientLineEdit->text().isEmpty())
    {
        ui->statusbar->showMessage("Caminho do ELEMENTCLIENT não informado!", 2000);
        return;
    }

    QItemSelectionModel *select = ui->charsTableView->selectionModel();
    if(select->hasSelection())
    {
        QModelIndex selectedIndex = select->selectedRows().first();
        const CharData &data = m_model->getCharData(selectedIndex.row());

        QStringList parameters{
            "startbypatcher",
            QString("user:%1").arg(data.getAccount()),
            QString("pwd:%1").arg(data.getPassword()),
            QString("role:%1").arg(data.getCharName()),
            ui->unfreezeCheckbox->isChecked() ? QString("rendernofocus") : ""
        };

        qint64 pid;
        QProcess* newProc = new QProcess();
        QString execPath = ui->elementClientLineEdit->text();
        QFileInfo fileInfo(execPath);
        newProc->startDetached(execPath, parameters, fileInfo.absolutePath(), &pid);
        m_pidList.push_back(pid);
    }
}

void MainWindow::deleteChar()
{
    QItemSelectionModel *select = ui->charsTableView->selectionModel();
    if(select->hasSelection())
    {
        QModelIndex selectedIndex = select->selectedRows().first();
        m_model->deleteCharData(selectedIndex.row());
    }
}
