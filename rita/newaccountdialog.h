#ifndef NEWACCOUNTDIALOG_H
#define NEWACCOUNTDIALOG_H

#include <QWidget>
#include <QDialog>
#include <QLineEdit>

class QDialogButtonBox;

class NewAccountDialog : public QDialog
{
    Q_OBJECT

public:
    NewAccountDialog(QList<QString> exList, QWidget *parent = 0);
    ~NewAccountDialog(){}

    QString getNewValue();

private:
    QLineEdit        *inputLine;
    QDialogButtonBox *buttonBox;
    QList<QString>    existingList;

//signals:
//    void rejected();
//    void accepted();

private slots:
   void validateValue();
};

#endif // NEWACCOUNTDIALOG_H
