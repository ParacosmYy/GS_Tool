#include "c16102/m16102.h"
QVector<double> m16102::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
