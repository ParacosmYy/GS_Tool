#include "m24992/m24992.h"
QVector<double> m24992::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
