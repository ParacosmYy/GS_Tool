#include "n25613/m25613.h"
QVector<double> m25613::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
