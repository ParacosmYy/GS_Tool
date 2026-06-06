#include "i9928/m9928.h"
QVector<double> m9928::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
