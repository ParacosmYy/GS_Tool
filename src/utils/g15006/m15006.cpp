#include "g15006/m15006.h"
QVector<double> m15006::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
