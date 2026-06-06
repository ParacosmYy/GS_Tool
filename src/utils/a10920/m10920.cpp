#include "a10920/m10920.h"
QVector<double> m10920::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
