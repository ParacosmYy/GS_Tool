#include "a7980/m7980.h"
QVector<double> m7980::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
