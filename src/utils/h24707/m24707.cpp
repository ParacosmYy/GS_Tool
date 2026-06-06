#include "h24707/m24707.h"
QVector<double> m24707::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
