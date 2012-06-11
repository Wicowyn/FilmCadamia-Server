#ifndef MAFENETRE_H
#define MAFENETRE_H

#include <QtGui>
#include <QtNetwork>
#include <QtSql>
#include "Client.h"
#include "GRAmis.h"

/*
A fair:
    -
*/

class MaFenetre : public QWidget{
    Q_OBJECT

public:
    MaFenetre(QWidget *parent = 0);
    ~MaFenetre();

private slots:
    void nouvelConnexion();
    void connection();

private:
    QTcpServer *server;
    QVector<Client*> clients;
    QLabel *info;
    QLineEdit *addr;
    QSpinBox *port;
    QSqlDatabase db;
    QMap<QString, GRAmis*> GPAmis;
};

#endif // MAFENETRE_H
