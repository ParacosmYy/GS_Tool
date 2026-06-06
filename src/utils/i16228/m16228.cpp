#include "i16228/m16228.h"
QVector<double> m16228::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
