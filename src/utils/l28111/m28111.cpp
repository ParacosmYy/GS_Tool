#include "l28111/m28111.h"
QVector<double> m28111::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
