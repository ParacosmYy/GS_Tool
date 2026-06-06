#include "p8475/m8475.h"
QVector<double> m8475::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
