#include "h10927/m10927.h"
QVector<double> m10927::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
