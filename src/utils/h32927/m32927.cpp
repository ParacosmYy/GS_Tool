#include "h32927/m32927.h"
QVector<double> m32927::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
