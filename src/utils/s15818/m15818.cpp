#include "s15818/m15818.h"
QVector<double> m15818::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
