#include "k15750/m15750.h"
QVector<double> m15750::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
