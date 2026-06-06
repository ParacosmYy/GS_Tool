#include "a22500/m22500.h"
QVector<double> m22500::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
