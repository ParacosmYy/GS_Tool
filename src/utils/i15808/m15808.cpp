#include "i15808/m15808.h"
QVector<double> m15808::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
