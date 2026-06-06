#include "k28350/m28350.h"
QVector<double> m28350::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
