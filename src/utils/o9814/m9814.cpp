#include "o9814/m9814.h"
QVector<double> m9814::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
