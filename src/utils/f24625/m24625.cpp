#include "f24625/m24625.h"
QVector<double> m24625::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
