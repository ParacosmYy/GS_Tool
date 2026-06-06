#include "p25675/m25675.h"
QVector<double> m25675::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
