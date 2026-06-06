#include "i25068/m25068.h"
QVector<double> m25068::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
