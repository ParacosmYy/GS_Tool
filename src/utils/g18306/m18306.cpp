#include "g18306/m18306.h"
QVector<double> m18306::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
