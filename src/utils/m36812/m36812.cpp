#include "m36812/m36812.h"
QVector<double> m36812::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
