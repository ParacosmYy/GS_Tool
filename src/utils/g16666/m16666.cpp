#include "g16666/m16666.h"
QVector<double> m16666::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
