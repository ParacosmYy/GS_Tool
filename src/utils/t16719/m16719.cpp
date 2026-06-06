#include "t16719/m16719.h"
QVector<double> m16719::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
