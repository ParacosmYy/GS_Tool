#include "a28360/m28360.h"
QVector<double> m28360::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
