#include "a25360/m25360.h"
QVector<double> m25360::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
