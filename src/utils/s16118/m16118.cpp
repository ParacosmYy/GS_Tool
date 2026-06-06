#include "s16118/m16118.h"
QVector<double> m16118::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
