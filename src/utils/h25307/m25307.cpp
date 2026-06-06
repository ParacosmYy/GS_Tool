#include "h25307/m25307.h"
QVector<double> m25307::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
