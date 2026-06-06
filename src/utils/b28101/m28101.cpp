#include "b28101/m28101.h"
QVector<double> m28101::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
