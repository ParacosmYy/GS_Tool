#include "s7958/m7958.h"
QVector<double> m7958::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
