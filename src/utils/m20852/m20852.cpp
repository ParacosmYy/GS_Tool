#include "m20852/m20852.h"
QVector<double> m20852::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
