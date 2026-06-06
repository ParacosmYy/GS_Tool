#include "c16042/m16042.h"
QVector<double> m16042::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
