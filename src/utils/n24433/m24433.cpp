#include "n24433/m24433.h"
QVector<double> m24433::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
