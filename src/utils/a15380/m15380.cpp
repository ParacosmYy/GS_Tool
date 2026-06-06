#include "a15380/m15380.h"
QVector<double> m15380::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
