#include "k9350/m9350.h"
QVector<double> m9350::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
