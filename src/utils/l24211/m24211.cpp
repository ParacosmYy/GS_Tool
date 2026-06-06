#include "l24211/m24211.h"
QVector<double> m24211::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
