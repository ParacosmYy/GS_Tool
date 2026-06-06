#include "g27206/m27206.h"
QVector<double> m27206::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
