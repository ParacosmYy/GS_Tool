#include "n10253/m10253.h"
QVector<double> m10253::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
