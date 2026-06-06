#include "l28051/m28051.h"
QVector<double> m28051::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
