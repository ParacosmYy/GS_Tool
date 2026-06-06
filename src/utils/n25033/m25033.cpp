#include "n25033/m25033.h"
QVector<double> m25033::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
