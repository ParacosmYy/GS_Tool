#include "h15887/m15887.h"
QVector<double> m15887::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
