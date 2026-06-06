#include "k35170/m35170.h"
QVector<double> m35170::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
