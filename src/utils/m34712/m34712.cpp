#include "m34712/m34712.h"
QVector<double> m34712::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
