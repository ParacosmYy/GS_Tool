#include "h35087/m35087.h"
QVector<double> m35087::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
