#include "t25379/m25379.h"
QVector<double> m25379::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
