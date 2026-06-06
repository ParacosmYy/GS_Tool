#include "s9818/m9818.h"
QVector<double> m9818::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
