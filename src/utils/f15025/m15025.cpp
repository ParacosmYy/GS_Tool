#include "f15025/m15025.h"
QVector<double> m15025::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
