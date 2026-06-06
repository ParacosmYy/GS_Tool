#include "m33052/m33052.h"
QVector<double> m33052::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
