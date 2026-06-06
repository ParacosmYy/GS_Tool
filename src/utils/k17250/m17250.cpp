#include "k17250/m17250.h"
QVector<double> m17250::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
