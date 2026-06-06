#include "k24530/m24530.h"
QVector<double> m24530::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
