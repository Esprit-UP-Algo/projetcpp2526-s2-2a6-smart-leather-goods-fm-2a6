#ifndef CLIENT_H
#define CLIENT_H

#include <QString>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlError>
#include <QDebug>

class Client
{
public:
    // Constructeurs
    Client();
    Client(QString cin, QString nom, QString tel, QString email, QString adresse);
    Client(int id, QString cin, QString nom, QString tel, QString email, QString adresse);

    // Getters
    int getId() const;
    QString getCin() const;
    QString getNom() const;
    QString getTel() const;
    QString getEmail() const;
    QString getAdresse() const;

    // Setters
    void setId(int id);
    void setCin(const QString& cin);
    void setNom(const QString& nom);
    void setTel(const QString& tel);
    void setEmail(const QString& email);
    void setAdresse(const QString& adresse);

    // Méthodes CRUD
    bool ajouter();
    bool supprimer(int id);
    bool modifier(int id);
    QSqlQueryModel* afficher();
    QSqlQueryModel* rechercher(const QString& critere);

private:
    int id;
    QString cin;
    QString nom;
    QString tel;
    QString email;
    QString adresse;
};

#endif // CLIENT_H   // ← Correction: ajout du commentaire
