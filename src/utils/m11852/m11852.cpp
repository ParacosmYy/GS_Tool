#include "m11852/m11852.h"
QVector<double> m11852::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
