#include "s16518/m16518.h"
QVector<double> m16518::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
