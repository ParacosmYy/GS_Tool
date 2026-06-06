#include "h15307/m15307.h"
QVector<double> m15307::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
