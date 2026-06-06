#include "g28106/m28106.h"
QVector<double> m28106::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
