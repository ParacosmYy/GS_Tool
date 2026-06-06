#include "k15530/m15530.h"
QVector<double> m15530::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
