#include "p8695/m8695.h"
QVector<double> m8695::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
