#include "m10812/m10812.h"
QVector<double> m10812::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
