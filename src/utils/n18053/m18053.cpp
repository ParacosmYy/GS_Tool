#include "n18053/m18053.h"
QVector<double> m18053::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
