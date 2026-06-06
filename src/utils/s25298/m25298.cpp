#include "s25298/m25298.h"
QVector<double> m25298::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
