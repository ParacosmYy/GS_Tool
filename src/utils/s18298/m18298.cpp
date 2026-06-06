#include "s18298/m18298.h"
QVector<double> m18298::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
