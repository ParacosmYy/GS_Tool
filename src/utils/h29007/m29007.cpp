#include "h29007/m29007.h"
QVector<double> m29007::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
