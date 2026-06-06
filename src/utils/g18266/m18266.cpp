#include "g18266/m18266.h"
QVector<double> m18266::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
