#include "s25058/m25058.h"
QVector<double> m25058::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
