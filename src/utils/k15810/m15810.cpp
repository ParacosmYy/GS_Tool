#include "k15810/m15810.h"
QVector<double> m15810::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
