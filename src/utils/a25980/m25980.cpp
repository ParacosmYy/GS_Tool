#include "a25980/m25980.h"
QVector<double> m25980::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
