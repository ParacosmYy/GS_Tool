#include "s35298/m35298.h"
QVector<double> m35298::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
