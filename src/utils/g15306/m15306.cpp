#include "g15306/m15306.h"
QVector<double> m15306::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
