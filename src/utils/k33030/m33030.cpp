#include "k33030/m33030.h"
QVector<double> m33030::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
