#include "h9927/m9927.h"
QVector<double> m9927::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
