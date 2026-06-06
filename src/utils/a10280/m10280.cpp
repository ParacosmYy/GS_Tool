#include "a10280/m10280.h"
QVector<double> m10280::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
