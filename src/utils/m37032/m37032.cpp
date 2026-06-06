#include "m37032/m37032.h"
QVector<double> m37032::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
