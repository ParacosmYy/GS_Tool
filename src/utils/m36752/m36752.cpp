#include "m36752/m36752.h"
QVector<double> m36752::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
