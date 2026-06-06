#include "a20580/m20580.h"
QVector<double> m20580::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
