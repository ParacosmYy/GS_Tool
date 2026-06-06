#include "m31812/m31812.h"
QVector<double> m31812::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
