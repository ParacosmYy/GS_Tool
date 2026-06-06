#include "m25752/m25752.h"
QVector<double> m25752::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
