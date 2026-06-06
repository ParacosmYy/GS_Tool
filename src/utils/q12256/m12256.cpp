#include "q12256/m12256.h"
QVector<double> m12256::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
