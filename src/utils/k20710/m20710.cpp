#include "k20710/m20710.h"
QVector<double> m20710::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
