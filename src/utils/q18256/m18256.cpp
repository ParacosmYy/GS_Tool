#include "q18256/m18256.h"
QVector<double> m18256::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
