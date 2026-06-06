#include "s9758/m9758.h"
QVector<double> m9758::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
