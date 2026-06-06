#include "g8486/m8486.h"
QVector<double> m8486::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
