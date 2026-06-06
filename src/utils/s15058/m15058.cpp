#include "s15058/m15058.h"
QVector<double> m15058::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
