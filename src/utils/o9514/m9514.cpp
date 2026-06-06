#include "o9514/m9514.h"
QVector<double> m9514::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
