#include "fileitem.h"

#include <QFileInfo>

FileItem::FileItem(const QIcon &icon, const QString &filePath,
    Dimension dim, QListWidget *parent) : QListWidgetItem(parent),
    m_filePath(filePath), m_dim(dim)
{
  setIcon(icon);
  QFileInfo fileInfo(m_filePath);
  setText(fileInfo.fileName());
}
