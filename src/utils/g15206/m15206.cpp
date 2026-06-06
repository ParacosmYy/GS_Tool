#include "g15206/m15206.h"
QVector<double> m15206::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
