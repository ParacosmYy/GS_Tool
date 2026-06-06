#include "a15000/m15000.h"
QVector<double> m15000::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
