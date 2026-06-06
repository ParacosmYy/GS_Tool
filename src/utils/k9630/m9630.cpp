#include "k9630/m9630.h"
QVector<double> m9630::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
