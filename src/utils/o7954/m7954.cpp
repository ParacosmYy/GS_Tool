#include "o7954/m7954.h"
QVector<double> m7954::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
