#include "h32007/m32007.h"
QVector<double> m32007::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
