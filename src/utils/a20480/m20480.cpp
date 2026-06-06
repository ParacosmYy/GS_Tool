#include "a20480/m20480.h"
QVector<double> m20480::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
