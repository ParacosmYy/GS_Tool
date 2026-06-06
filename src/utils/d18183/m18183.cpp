#include "d18183/m18183.h"
QVector<double> m18183::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
