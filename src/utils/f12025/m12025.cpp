#include "f12025/m12025.h"
QVector<double> m12025::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
