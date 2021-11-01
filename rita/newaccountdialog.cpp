#include "newaccountdialog.h"
#include <QDialogButtonBox>
#include <QRegExpValidator>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>


NewAccountDialog::NewAccountDialog(QList<QString> exList, QWidget *parent){

    Qt::WindowFlags WTOPFLAGS_D
                = Qt::Dialog
                | Qt::CustomizeWindowHint
                | Qt::WindowTitleHint
                | Qt::WindowCloseButtonHint
                | Qt::WindowStaysOnTopHint
                ;


    setWindowFlags(WTOPFLAGS_D);
    setWindowTitle(tr("New user"));

    inputLine = 0;
    existingList = exList;

    setAttribute(Qt::WA_QuitOnClose, false);

    QVBoxLayout * vbox = new QVBoxLayout;

    QLabel *ulbl= new QLabel(tr("User login:"));
    inputLine   = new QLineEdit();
    inputLine->setPlaceholderText(tr("Input user name"));


    ulbl->setPixmap( QPixmap(":/img/images/add_user.png") );

    vbox->addWidget( ulbl );
    vbox->addWidget(inputLine);

    QRegExpValidator * v
            = new QRegExpValidator(QRegExp("[\\da-zA-Z_]{24}"));

    inputLine->setValidator(v);

    buttonBox= new QDialogButtonBox( QDialogButtonBox::Ok
                                    | QDialogButtonBox::Cancel);

    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    QPushButton *clButton = buttonBox->button(QDialogButtonBox::Cancel);

    okButton->setEnabled(false);
    okButton->setText(tr("&Create"));
    clButton->setText(tr("Cancel"));

    connect(okButton,  SIGNAL(pressed()),  this, SLOT(accept()));
    connect(clButton,  SIGNAL(pressed()),  this, SLOT(reject()));
    connect(inputLine, SIGNAL(textChanged(QString)),
                                            this, SLOT(validateValue()));

    vbox->addWidget(buttonBox);
    setLayout(vbox);
}


QString NewAccountDialog::getNewValue(){

    return inputLine->text();
}

void NewAccountDialog::validateValue(){

    buttonBox->button(QDialogButtonBox::Ok)
                ->setEnabled(
                    !getNewValue().isEmpty()
                 && !existingList.contains( getNewValue().toLower() ) );
}
