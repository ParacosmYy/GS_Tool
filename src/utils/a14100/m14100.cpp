#include "a14100/m14100.h"
QVector<double> m14100::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
