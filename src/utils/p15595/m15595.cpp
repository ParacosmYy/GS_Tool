#include "p15595/m15595.h"
QVector<double> m15595::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
