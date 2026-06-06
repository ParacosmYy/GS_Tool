#include "h25507/m25507.h"
QVector<double> m25507::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
