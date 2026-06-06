#include "i17328/m17328.h"
QVector<double> m17328::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
