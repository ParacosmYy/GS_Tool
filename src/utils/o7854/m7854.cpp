#include "o7854/m7854.h"
QVector<double> m7854::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
