#ifndef GRAMIS_H
#define GRAMIS_H

#include "Client.h"
#include <QtGui>
#include <QtSql>
class Client;

// PUTIN DE BORDEL DE MERDE un jour je trouverais...

/*
  A améliorer:
  -lors d'une connection ne pas envoyer tout la liste
   mais juste la personne co/deco pour économie de BP
  -protecion contre multi-tentative d'accés

  Autre mode de fonctionnement:
  -connect au groupe sous condition de login/password valide
        Avantage:
        -plus de fonction join()
        Inconvénient:
        -en cas de kick mémo de infos
        -save des login par le client (conso mémoire)
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
