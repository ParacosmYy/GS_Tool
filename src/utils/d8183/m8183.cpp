#include "d8183/m8183.h"
QVector<double> m8183::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
