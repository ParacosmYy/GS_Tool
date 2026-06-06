#include "m15012/m15012.h"
QVector<double> m15012::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
