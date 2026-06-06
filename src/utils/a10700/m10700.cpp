#include "a10700/m10700.h"
QVector<double> m10700::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
