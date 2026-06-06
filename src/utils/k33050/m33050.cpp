#include "k33050/m33050.h"
QVector<double> m33050::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
