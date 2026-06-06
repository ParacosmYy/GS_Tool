#include "s8558/m8558.h"
QVector<double> m8558::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
