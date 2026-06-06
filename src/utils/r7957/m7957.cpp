#include "r7957/m7957.h"
QVector<double> m7957::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
