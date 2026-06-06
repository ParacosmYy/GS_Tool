#include "t35059/m35059.h"
QVector<double> m35059::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
