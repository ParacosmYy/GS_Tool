#include "n25193/m25193.h"
QVector<double> m25193::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
