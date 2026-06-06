#include "t9939/m9939.h"
QVector<double> m9939::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
