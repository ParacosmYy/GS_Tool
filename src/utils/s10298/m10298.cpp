#include "s10298/m10298.h"
QVector<double> m10298::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
