#include "i17408/m17408.h"
QVector<double> m17408::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
