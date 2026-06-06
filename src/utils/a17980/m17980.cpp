#include "a17980/m17980.h"
QVector<double> m17980::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
