#include "mafenetre.h"

MaFenetre::MaFenetre(QWidget *parent) : QWidget(parent){
    qDebug()<<endl;

    info=new QLabel(this);
    info->setWordWrap(true);
    port=new QSpinBox(this);
    port->setRange(1, 65535);
    port->setValue(29000);
    addr=new QLineEdit("localhost", this);

    QFormLayout *layoutF=new QFormLayout;
    layoutF->addRow("Port d'écoute", port);
    layoutF->addRow("IP de la BDD", addr);

    QVBoxLayout *layout=new QVBoxLayout(this);
    QPushButton *quitter=new QPushButton("Quitter", this);
    QPushButton *connection=new QPushButton("Connection", this);
    layout->addLayout(layoutF);
    layout->addWidget(connection);
    layout->addWidget(quitter);
    layout->addWidget(info);

    setLayout(layout);

    server=new QTcpServer(this);
    db=QSqlDatabase::addDatabase("QMYSQL");

    connect(server, SIGNAL(newConnection()), this, SLOT(nouvelConnexion()));
    connect(connection, SIGNAL(clicked()), this, SLOT(connection()));
    connect(quitter, SIGNAL(clicked()), qApp, SLOT(quit()));
}

void MaFenetre::connection(){
    QString str;
    server->close();
    db.close();
    if(server->listen(QHostAddress::Any, port->value())) str="Le server est démarrer sur le port "+QString::number(server->serverPort());
    else str=server->errorString();
    str+="\n\n";

    db.setHostName(addr->text());
    db.setDatabaseName("FilmCadamia");
    db.setUserName("yapiti");
    db.setPassword("7sx6q8fs");
    if(db.open()) str+="Base de donnée ouverte à l'adresse "+db.hostName();
    else str+=db.lastError().text();
    info->setText(str);
    adjustSize();
}

MaFenetre::~MaFenetre(){
    db.close();
}

void MaFenetre::nouvelConnexion(){
    clients.append(new Client(server->nextPendingConnection(), GPAmis, this));
}
