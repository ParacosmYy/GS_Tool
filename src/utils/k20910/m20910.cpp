#include "k20910/m20910.h"
QVector<double> m20910::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
