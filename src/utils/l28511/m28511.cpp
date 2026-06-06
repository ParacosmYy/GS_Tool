#include "l28511/m28511.h"
QVector<double> m28511::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
