#include "k33130/m33130.h"
QVector<double> m33130::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
