#include "h17007/m17007.h"
QVector<double> m17007::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
