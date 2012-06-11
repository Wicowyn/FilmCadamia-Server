#include "GRAmis.h"

GRAmis::GRAmis(QString str, QMap<QString, GRAmis*>& map, QObject *parent) : QObject(parent), GPAmis(map){
    QSqlQuery query;
    query.exec("SELECT * FROM Groupe WHERE login='"+str+"'");
    query.next();
    login=query.value(1).toString();
    password=query.value(2).toString();
    adepte=query.value(3).toString().split("#");
    adepte.removeAt(adepte.indexOf(QString())); //lorsque le groupe est vide on récupére un utilisateur : "", donc on le vire;
}

GRAmis::~GRAmis(){
    qDebug()<<"Deconnection du GP:"<<login;
    GPAmis.remove(login);
}

/*QMap<QString, QString> GRAmis::allInfoFilmOf(QString name){
    return adepteCo[name]->allInfoFilm();
}*/

bool GRAmis::contains(QString name){
    return adepte.contains(name);
}

bool GRAmis::join(Client *client, QString mdp){
    qDebug()<<"GP:"<<login<<", tentative de connection avec:"<<mdp<<", good"<<password;
    if(mdp!=password || adepte.contains(client->name())) return false;
    adepte.append(client->name());
    saveListMembre();
    connect(client);
    return true;
}

bool GRAmis::connect(Client *client){
    if(adepte.contains(client->name())){
        for(QMap<QString, Client*>::iterator it=adepteCo.begin(); it!=adepteCo.end(); it++) it.value()->sendUser(client->name(), true);
        adepteCo[client->name()]=client;
        return true;
    }
    else return false;
}

void GRAmis::disconnect(Client *client){
    adepteCo.erase(adepteCo.find(client->name()));
    for(QMap<QString, Client*>::iterator it=adepteCo.begin(); it!=adepteCo.end(); it++) it.value()->sendUser(client->name(), false);
    if(adepteCo.count()==0) deleteLater();
}

QMap<QString, bool> GRAmis::getList(){
    QMap<QString, bool> map;
    for(int i=0; i<adepte.count(); i++) map[adepte[i]]=false;
    for(QMap<QString, Client*>::iterator it=adepteCo.begin(); it!=adepteCo.end(); it++) map[it.value()->name()]=true;

    return map;
}

QStringList GRAmis::getAdepte(){
    return adepte;
}

QString GRAmis::name(){
    return login;
}

bool GRAmis::saveListMembre(){
    QSqlQuery query;
    qDebug()<<adepte;
    return query.exec("UPDATE Groupe SET adepte='"+adepte.join("#")+"' WHERE login='"+login+"'");
}

bool GRAmis::createGP(QString name, QString mdp){
    QSqlQuery query;
    return query.exec("INSERT INTO Groupe (login, password) VALUES ('"+name+"', '"+mdp+"')");
}

bool GRAmis::exists(QString name){
    QSqlQuery query;
    query.exec("SELECT login FROM Groupe WHERE login='"+name+"'");
    return query.next();
}
