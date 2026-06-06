#include "e15204/m15204.h"
QVector<double> m15204::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
