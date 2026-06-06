#include "h8007/m8007.h"
QVector<double> m8007::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
