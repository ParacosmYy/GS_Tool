#include "m13852/m13852.h"
QVector<double> m13852::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
