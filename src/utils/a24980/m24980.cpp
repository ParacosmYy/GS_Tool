#include "a24980/m24980.h"
QVector<double> m24980::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
