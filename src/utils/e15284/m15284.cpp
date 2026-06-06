#include "e15284/m15284.h"
QVector<double> m15284::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
