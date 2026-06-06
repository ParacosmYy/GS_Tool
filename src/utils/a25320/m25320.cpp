#include "a25320/m25320.h"
QVector<double> m25320::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
