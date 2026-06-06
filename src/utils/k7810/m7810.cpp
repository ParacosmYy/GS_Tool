#include "k7810/m7810.h"
QVector<double> m7810::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
