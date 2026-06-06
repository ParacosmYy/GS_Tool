#include "h24207/m24207.h"
QVector<double> m24207::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
