#include "q32256/m32256.h"
QVector<double> m32256::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
