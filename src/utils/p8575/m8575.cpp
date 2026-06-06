#include "p8575/m8575.h"
QVector<double> m8575::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
