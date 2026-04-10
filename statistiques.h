#ifndef STATISTIQUES_H
#define STATISTIQUES_H

#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>

class Statistiques : public QDialog
{
    Q_OBJECT

public:
    Statistiques(QWidget *parent = nullptr);
    ~Statistiques();
};

#endif
