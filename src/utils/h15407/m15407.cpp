#include "h15407/m15407.h"
QVector<double> m15407::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
