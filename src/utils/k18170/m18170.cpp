#include "k18170/m18170.h"
QVector<double> m18170::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
