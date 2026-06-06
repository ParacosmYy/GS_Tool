#include "g8746/m8746.h"
QVector<double> m8746::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
