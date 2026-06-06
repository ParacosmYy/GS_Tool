#include "k14010/m14010.h"
QVector<double> m14010::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
