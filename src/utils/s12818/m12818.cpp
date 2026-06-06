#include "s12818/m12818.h"
QVector<double> m12818::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
