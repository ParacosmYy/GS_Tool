#include "m8852/m8852.h"
QVector<double> m8852::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
