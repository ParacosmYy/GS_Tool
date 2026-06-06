#include "a31500/m31500.h"
QVector<double> m31500::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
