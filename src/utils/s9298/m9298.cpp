#include "s9298/m9298.h"
QVector<double> m9298::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
