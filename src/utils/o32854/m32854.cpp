#include "o32854/m32854.h"
QVector<double> m32854::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
