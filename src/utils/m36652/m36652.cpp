#include "m36652/m36652.h"
QVector<double> m36652::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
