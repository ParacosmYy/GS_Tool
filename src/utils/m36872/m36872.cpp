#include "m36872/m36872.h"
QVector<double> m36872::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
