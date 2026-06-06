#include "a15980/m15980.h"
QVector<double> m15980::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
