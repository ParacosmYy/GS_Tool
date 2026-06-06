#include "g15706/m15706.h"
QVector<double> m15706::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
