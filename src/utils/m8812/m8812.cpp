#include "m8812/m8812.h"
QVector<double> m8812::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
