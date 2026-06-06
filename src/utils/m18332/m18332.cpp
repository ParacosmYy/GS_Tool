#include "m18332/m18332.h"
QVector<double> m18332::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
