#include "m27812/m27812.h"
QVector<double> m27812::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
