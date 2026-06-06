#include "t25339/m25339.h"
QVector<double> m25339::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
