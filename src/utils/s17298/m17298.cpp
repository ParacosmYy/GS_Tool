#include "s17298/m17298.h"
QVector<double> m17298::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
