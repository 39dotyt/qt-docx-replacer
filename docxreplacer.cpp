#include "docxreplacer.h"

#include <QDir>
#include <QFile>
#include <QStringList>
#include <QTextStream>
#include <QDebug>
#include "quazip/JlCompress.h"

bool DocxReplacer::replaceInFile(
    QString fileName, QMap<QString, QString> *replaceRules,
    QString newFileName) {
  QString tempDirPath = QDir::tempPath() + "/docx-replacer";
  if (!removeFolder(tempDirPath) ||
      JlCompress::extractDir(fileName, tempDirPath).isEmpty()) {
    qCritical() << "Cannot remove folder or extract docx";
    return false;
  }
  QFile file(tempDirPath + "/word/document.xml");
  QTextStream stream;
  stream.setDevice(&file);
  if (!file.open(QIODevice::ReadOnly)) {
    qCritical() << "Cannon open file for reading";
    return false;
  }
  QString content = stream.readAll();
  file.close();
  QMap<QString, QString>::const_iterator replaceRulesIterator =
      replaceRules->constBegin();
  while (replaceRulesIterator != replaceRules->constEnd()) {
    content.replace(replaceRulesIterator.key(), replaceRulesIterator.value());
    ++replaceRulesIterator;
  }
  if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    qCritical() << "Cannot open file for writing";
    return false;
  }
  stream << content;
  file.close();
  if (!newFileName.isEmpty()) {
    fileName = newFileName;
  }
  return JlCompress::compressDir(fileName, tempDirPath);
}

bool DocxReplacer::removeFolder(QString path) {
  QDir dir(path);
  if (!dir.exists()) {
    return true;
  }
  QStringList files = dir.entryList(QDir::Files | QDir::Hidden);
  QString removablePath;
  for (int i = 0; i < files.count(); ++i) {
    removablePath = path + "/" + files.at(i);
    if (!QFile::remove(removablePath)) {
      qCritical() << "Cannot remove file" << removablePath;
      return false;
    }
  }
  files.clear();
  files = dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
  for (int i = 0; i < files.count(); ++i) {
    removablePath = path + "/" + files.at(i);
    if (!removeFolder(removablePath)) {
      qCritical() << "Cannot remove directory" << removablePath;
      return false;
    }
  }
  return dir.rmdir(path);
}
