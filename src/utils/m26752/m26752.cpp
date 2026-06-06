#include "m26752/m26752.h"
QVector<double> m26752::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
