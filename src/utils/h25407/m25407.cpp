#include "h25407/m25407.h"
QVector<double> m25407::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
