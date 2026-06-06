#include "k15570/m15570.h"
QVector<double> m15570::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
