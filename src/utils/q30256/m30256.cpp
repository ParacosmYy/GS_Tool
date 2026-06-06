#include "q30256/m30256.h"
QVector<double> m30256::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
