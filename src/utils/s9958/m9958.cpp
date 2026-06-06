#include "s9958/m9958.h"
QVector<double> m9958::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
