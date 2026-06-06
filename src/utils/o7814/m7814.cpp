#include "o7814/m7814.h"
QVector<double> m7814::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
