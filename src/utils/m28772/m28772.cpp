#include "m28772/m28772.h"
QVector<double> m28772::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
