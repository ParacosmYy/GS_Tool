#include "h28527/m28527.h"
QVector<double> m28527::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
