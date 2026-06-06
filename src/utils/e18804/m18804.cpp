#include "e18804/m18804.h"
QVector<double> m18804::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
