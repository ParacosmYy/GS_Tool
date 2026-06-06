#include "m32812/m32812.h"
QVector<double> m32812::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
