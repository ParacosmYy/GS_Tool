#include "k29270/m29270.h"
QVector<double> m29270::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
