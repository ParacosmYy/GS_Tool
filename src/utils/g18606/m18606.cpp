#include "g18606/m18606.h"
QVector<double> m18606::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
