#include "m17752/m17752.h"
QVector<double> m17752::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
