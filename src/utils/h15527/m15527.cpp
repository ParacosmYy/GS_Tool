#include "h15527/m15527.h"
QVector<double> m15527::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
