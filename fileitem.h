#ifndef FILEITEM_H
#define FILEITEM_H

#include <QListWidgetItem>

class FileItem : public QListWidgetItem
{
  public:
    enum Dimension {
      Dim0D,
      Dim2D,
      Dim3D
    };

    FileItem(const QIcon &icon, const QString &filePath, Dimension dim, QListWidget *parent = 0);

    bool is3D() const
    {
      return (m_dim == 3);
    }

    bool is2D() const
    {
      return (m_dim == 2);
    }

    QString filePath() const
    {
      return m_filePath;
    }

  private:
    QString m_filePath;
    int m_dim;
};

#endif // FILEITEM_H
