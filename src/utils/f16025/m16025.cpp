#include "f16025/m16025.h"
QVector<double> m16025::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
