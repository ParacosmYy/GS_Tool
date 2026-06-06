#include "m18852/m18852.h"
QVector<double> m18852::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
