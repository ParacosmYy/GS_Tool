#include "a20800/m20800.h"
QVector<double> m20800::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
