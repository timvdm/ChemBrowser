#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QModelIndex>

namespace Ui
{
    class MainWindow;
}

namespace OpenBabel {
  class OBConversion;
}

namespace Avogadro {
  class GLWidget;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    /**
     * Clear the list widget with content.
     */
    void clearContents();

    QPixmap getImage2D(const QString &fileName);
    QPixmap getImage3D(const QString &fileName);

  protected:
    void hideEvent(QHideEvent *event);

  private Q_SLOTS:
    void updateContents(QModelIndex);
  private:
    Ui::MainWindow *ui;
    QString m_lastPath;
    Avogadro::GLWidget *m_glWidget;
    OpenBabel::OBConversion *m_conv;
};

#endif // MAINWINDOW_H
