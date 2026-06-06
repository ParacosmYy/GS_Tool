#include "f24025/m24025.h"
QVector<double> m24025::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
