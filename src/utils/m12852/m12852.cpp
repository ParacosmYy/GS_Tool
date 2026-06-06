#include "m12852/m12852.h"
QVector<double> m12852::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
