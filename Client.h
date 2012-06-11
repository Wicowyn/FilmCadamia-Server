#ifndef CLIENT_H
#define CLIENT_H

#include "GRAmis.h"
#include "../../command.h"
#include <QtGui>
#include <QtNetwork>
#include <QtSql>

class GRAmis;

/*
  A fair:
    -protection contre les multiples tentatives de connection
    -penser à notifier le groupe d'une MAJ de library ==> ou pas oublie pas la règle
  Règle à tenir:
    -une demande -> une action
*/


class Client : public QObject{
    Q_OBJECT
public:
    Client(QTcpSocket*, QMap<QString, GRAmis*>& lol, QObject *parent = 0);
    ~Client();
    QString name();
    void sendUser(QString, bool);
    void sendAllGroup();
    //void sendAllInfoFilmOf(QString);
    void updateLibrary(QVector<int> data);
    QMap <QString, QString> allInfoFilm();
    bool saveListGP();
    void createAccount(UserInfo);
    static bool createMembre(UserInfo);
    static bool exists(QString name);
    static QList<int> getLibraryOf(QString);
    static int getVersionOf(QString);
    void checkFriend(QMap<QString, int>);

signals:
    void connected();

private slots:
    void newData();

private:
    void trialConnect();
    void createGP(QString, QString);
    void joinGP(QString, QString);
    QString login, password, IP, nom, prenom, eMail;
    QTcpSocket *socket;
    //QMap<QString, QString> library;
    QMap<QString, GRAmis*>& GPAmis;
    QMap<QString, GRAmis*> myGroup;
    bool connecter;
};

#endif // CLIENT_H
