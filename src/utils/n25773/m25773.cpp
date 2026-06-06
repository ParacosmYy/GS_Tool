#include "n25773/m25773.h"
QVector<double> m25773::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
