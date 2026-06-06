#include "b15881/m15881.h"
QVector<double> m15881::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
