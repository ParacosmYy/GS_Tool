#include "a17200/m17200.h"
QVector<double> m17200::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
