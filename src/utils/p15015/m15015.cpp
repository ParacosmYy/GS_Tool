#include "p15015/m15015.h"
QVector<double> m15015::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
