#include "m36632/m36632.h"
QVector<double> m36632::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
