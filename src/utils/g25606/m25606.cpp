#include "g25606/m25606.h"
QVector<double> m25606::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
