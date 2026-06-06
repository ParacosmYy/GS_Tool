#include "h9007/m9007.h"
QVector<double> m9007::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
