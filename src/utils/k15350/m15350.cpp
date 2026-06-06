#include "k15350/m15350.h"
QVector<double> m15350::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
