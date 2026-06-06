#include "m25992/m25992.h"
QVector<double> m25992::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
