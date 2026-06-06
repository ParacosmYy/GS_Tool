#include "p15555/m15555.h"
QVector<double> m15555::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
