#include "a27100/m27100.h"
QVector<double> m27100::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
