#include "l28031/m28031.h"
QVector<double> m28031::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
