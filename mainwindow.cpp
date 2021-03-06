#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileSystemModel>
#include <QPainter>
#include <QSettings>
#include <QGLFramebufferObject>
#include <QProcess>
#include <QDebug>

#include <openbabel/obconversion.h>
#include <openbabel/mol.h>

#include <Molsketch/Molecule>
#include <Molsketch/FileIO>
#include <Molsketch/MolScene>

#include <avogadro/molecule.h>
#include <avogadro/glwidget.h>
#include <avogadro/moleculefile.h>

#include "fileitem.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  // create the QFileSystemModel
  QFileSystemModel *model = new QFileSystemModel;
  model->setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

  QSettings settings;
  QString path = settings.value("path", "/home").toString();
  model->setRootPath(path);
  ui->fileView->setModel(model);
  connect(ui->fileView, SIGNAL(clicked(QModelIndex)), this, SLOT(updateContents(QModelIndex)));

  // set the content view options
  ui->listWidget->setIconSize(QSize(128, 128));
  ui->listWidget->setViewMode(QListWidget::IconMode);

  m_glWidget = new Avogadro::GLWidget;
  m_glWidget->setBackground(Qt::white);
  m_glWidget->loadDefaultEngines();
  m_glWidget->show();
  m_glWidget->setGeometry(10000,10000,320,240); // ugly hack...
  //m_glWidget->hide();



  connect(ui->listWidget, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(openItem(QModelIndex)));

}

MainWindow::~MainWindow()
{  
  QSettings settings;
  QFileSystemModel *model = static_cast<QFileSystemModel*>(ui->fileView->model());
  settings.setValue("path", m_lastPath);
//  delete m_glWidget;
  delete ui;
}

void MainWindow::hideEvent(QHideEvent *event)
{
  delete m_glWidget;
  QMainWindow::hideEvent(event);
}

QPixmap MainWindow::getImage2D(const QString &fileName)
{
  if (fileName.endsWith(".png"))
    return QPixmap(fileName);

  bool validFormat = false;
  if (fileName.endsWith(".cml")) validFormat = true;
  if (fileName.endsWith(".sdf")) validFormat = true;
  if (fileName.endsWith(".mol")) validFormat = true;
  if (fileName.endsWith(".mol2")) validFormat = true;

  if (!validFormat)
    return QPixmap();

  Molsketch::Molecule *molecule = Molsketch::loadFile(fileName);
  if (!molecule)
    return QPixmap();

  // Creating pixmap
  /* FIXME This doesn't show yet methane & consorts */
  /* TODO Perhaps we should resize the bond width and font based on the scale */
  Molsketch::MolScene renderScene;
  //renderScene.setRenderMode(Molsketch::MolScene::RenderColoredSquares);
  renderScene.addItem(molecule);
  renderScene.setChargeVisible(false);
  //renderScene.setAtomSymbolFont(QFont("Helvetica", 40));
  //renderScene.setBondWidth(5);
  // set the sceneRect to the items bouding rectangle
//  renderScene.setSceneRect(renderScene.itemsBoundingRect());
  renderScene.setSceneRect(molecule->boundingRect());

  // use a pixmap because we'll be using Qt::white in Atom::paint(...)
  QPixmap pixmap(renderScene.width(), renderScene.height());
  /*  TODO Perhaps the icons should all be the same size... */
  pixmap.fill();

  // Creating and setting the painter
  QPainter painter(&pixmap);
  painter.setRenderHint(QPainter::Antialiasing);
  renderScene.render(&painter);

  return pixmap;
}

QPixmap MainWindow::getImage3D(const QString &fileName)
{
  m_glWidget->show();

  //while (!m_glWidget->isVisible()) {}

  Avogadro::Molecule *molecule = Avogadro::MoleculeFile::readMolecule(fileName);
  if (!molecule) {
    //m_glWidget->hide();
    return QPixmap();
  }

  bool validFormat = false;
  if (fileName.endsWith("pdb")) validFormat = true;
  if (fileName.endsWith("xyz")) validFormat = true;
  if (fileName.endsWith("cml")) validFormat = true;


  if (!validFormat) {
    //m_glWidget->hide();
    return QPixmap();
  }

  qDebug() << "1";
  m_glWidget->setMolecule(molecule);


  qDebug() << "creating 3D image";
  QPixmap exportImage;

  m_glWidget->makeCurrent();
  m_glWidget->raise();
  m_glWidget->repaint();
  if (QGLFramebufferObject::hasOpenGLFramebufferObjects()) {
    QImage img = m_glWidget->grabFrameBuffer( true );
    exportImage = QPixmap::fromImage(img);
  } else {
    exportImage = QPixmap::grabWindow( m_glWidget->winId() );
  }
  qDebug() << "done.";
  //m_glWidget->hide();

  return exportImage;
}

void MainWindow::updateContents(QModelIndex index)
{
  // resize the first column to fit the contents
  ui->fileView->resizeColumnToContents(0);
  qDebug() << "updateContent";
  QFileSystemModel *model = static_cast<QFileSystemModel*>(ui->fileView->model());
  if (model->isDir(index)) {
    // clear previous content
    ui->listWidget->clear();
    m_lastPath = model->filePath(index);

    // add the new files
    QDir dir(model->filePath(index));
    dir.setFilter(QDir::Files);
    foreach (const QFileInfo &fileInfo, dir.entryInfoList()) {
      if (fileInfo.suffix().isEmpty())
        continue;
      // handle png images as a special case (todo: compute if image is possible molecule)
      if (fileInfo.suffix() == "png") {
        new QListWidgetItem(QPixmap(fileInfo.absoluteFilePath()), fileInfo.fileName(), ui->listWidget);
        continue;
      }

      QProcess process;
      QStringList arguments;
      arguments << fileInfo.absoluteFilePath();
      arguments << "-oxyz";
      process.start("babel", arguments);
      process.waitForFinished();
      if (process.exitCode()) {
        qDebug() << "skipping " << fileInfo.absoluteFilePath() << " bacause babel did not exit cleanly.";
        continue;
      } else {
        qDebug() << "babel clean exit for " << fileInfo.absoluteFilePath();
      }

      QString fileName = fileInfo.absoluteFilePath();
      bool validFormat = false;
      if (fileName.endsWith("pdb")) validFormat = true;
      if (fileName.endsWith("xyz")) validFormat = true;
      if (fileName.endsWith("cml")) validFormat = true;
      if (fileName.endsWith("mol")) validFormat = true;
      if (fileName.endsWith("mol2")) validFormat = true;

      if (!validFormat)
        continue;
 


      m_conv = new OpenBabel::OBConversion;
      if (!m_conv->SetInFormat(fileInfo.suffix().toAscii()))
        continue;

      OpenBabel::OBMol mol;
      std::ifstream ifs;
      ifs.open(fileInfo.absoluteFilePath().toAscii().data());
      if (!ifs) {
        delete m_conv;
        continue;
      }
      if (!m_conv->Read(&mol, &ifs)) {
        delete m_conv;
        continue;
      }
      ifs.close();

      delete m_conv;

      QPixmap icon;
      switch (mol.GetDimension()) {
        case 2:
          icon = getImage2D(fileInfo.absoluteFilePath());
          break;
        case 3:
          icon = getImage3D(fileInfo.absoluteFilePath());
          break;
      }

      if (!icon.isNull()) {
        FileItem *item = new FileItem(icon, fileInfo.absoluteFilePath(), static_cast<FileItem::Dimension>(mol.GetDimension()), ui->listWidget);
      }

      qDebug() << fileInfo.absoluteFilePath();
      qDebug() << fileInfo.suffix();
    }
  }
}

void MainWindow::openItem(QModelIndex index)
{
  FileItem *item = static_cast<FileItem*>(ui->listWidget->currentItem());
  if (!item) {
    qDebug() << "Could not open item...";
    return;
  }

  QString program;
  if (item->is2D())
    program = "molsketch";
  if (item->is3D())
    program = "avogadro";

  QStringList args;
  args << item->filePath();

  QProcess::execute(program, args);
}
