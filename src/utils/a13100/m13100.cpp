#include "a13100/m13100.h"
QVector<double> m13100::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
