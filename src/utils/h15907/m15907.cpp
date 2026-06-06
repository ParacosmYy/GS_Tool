#include "h15907/m15907.h"
QVector<double> m15907::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
