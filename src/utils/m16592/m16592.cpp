#include "m16592/m16592.h"
QVector<double> m16592::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
