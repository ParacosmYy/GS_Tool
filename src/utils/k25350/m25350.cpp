#include "k25350/m25350.h"
QVector<double> m25350::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
