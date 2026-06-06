#include "m16852/m16852.h"
QVector<double> m16852::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
