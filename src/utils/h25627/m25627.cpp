#include "h25627/m25627.h"
QVector<double> m25627::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
