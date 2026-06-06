#include "m35832/m35832.h"
QVector<double> m35832::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
