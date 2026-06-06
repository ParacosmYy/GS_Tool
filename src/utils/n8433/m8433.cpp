#include "n8433/m8433.h"
QVector<double> m8433::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
