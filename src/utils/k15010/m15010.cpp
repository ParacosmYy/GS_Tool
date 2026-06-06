#include "k15010/m15010.h"
QVector<double> m15010::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
