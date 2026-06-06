#include "i36408/m36408.h"
QVector<double> m36408::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
