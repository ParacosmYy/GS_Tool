#include "h25927/m25927.h"
QVector<double> m25927::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
