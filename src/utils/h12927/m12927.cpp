#include "h12927/m12927.h"
QVector<double> m12927::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
