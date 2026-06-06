#include "q15256/m15256.h"
QVector<double> m15256::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
