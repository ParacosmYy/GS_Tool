#include "k30170/m30170.h"
QVector<double> m30170::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
