#include "m18812/m18812.h"
QVector<double> m18812::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
