#include "m24852/m24852.h"
QVector<double> m24852::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
