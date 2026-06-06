#include "m13812/m13812.h"
QVector<double> m13812::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
