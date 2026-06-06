#include "e9544/m9544.h"
QVector<double> m9544::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
