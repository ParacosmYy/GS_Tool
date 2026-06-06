#include "i28528/m28528.h"
QVector<double> m28528::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
