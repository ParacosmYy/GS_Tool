#include "s8298/m8298.h"
QVector<double> m8298::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
