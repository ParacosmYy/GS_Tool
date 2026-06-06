#include "s15298/m15298.h"
QVector<double> m15298::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
