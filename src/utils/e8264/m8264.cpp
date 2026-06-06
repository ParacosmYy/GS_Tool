#include "e8264/m8264.h"
QVector<double> m8264::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
