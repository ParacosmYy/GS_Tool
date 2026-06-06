#include "g15406/m15406.h"
QVector<double> m15406::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
