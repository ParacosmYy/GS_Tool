#include "l28211/m28211.h"
QVector<double> m28211::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
