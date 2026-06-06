#include "o9494/m9494.h"
QVector<double> m9494::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
