#ifndef GRAMIS_H
#define GRAMIS_H

#include "Client.h"
#include <QtGui>
#include <QtSql>
class Client;

// PUTIN DE BORDEL DE MERDE un jour je trouverais...

/*
  A am�liorer:
  -lors d'une connection ne pas envoyer tout la liste
   mais juste la personne co/deco pour �conomie de BP
  -protecion contre multi-tentative d'acc�s

  Autre mode de fonctionnement:
  -connect au groupe sous condition de login/password valide
        Avantage:
        -plus de fonction join()
        Inconv�nient:
        -en cas de kick m�mo de infos
        -save des login par le client (conso m�moire)
        plop
*/

class GRAmis : public QObject{
    Q_OBJECT
public:
    GRAmis(QString, QMap<QString, GRAmis*>&, QObject *parent=0);
    ~GRAmis();
    bool connect(Client*);
    void disconnect(Client*);
    bool join(Client*, QString);
    bool contains(QString);
    bool saveListMembre();
    //QMap<QString, QString> allInfoFilmOf(QString);
    QMap<QString, bool> getList();
    QStringList getAdepte();
    QString name();
    //static bool requestJoin(QString, )
    static bool createGP(QString, QString);
    static bool exists(QString);

signals:
    void newConnection();

private:
    QString login, password;
    QStringList adepte;
    QMap<QString, Client*> adepteCo;
    QMap<QString, GRAmis*> &GPAmis;
};

#endif // GRAMIS_H
