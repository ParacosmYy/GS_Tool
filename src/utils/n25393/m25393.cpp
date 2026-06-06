#include "n25393/m25393.h"
QVector<double> m25393::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
