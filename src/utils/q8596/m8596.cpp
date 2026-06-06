#include "q8596/m8596.h"
QVector<double> m8596::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
