#include "s8958/m8958.h"
QVector<double> m8958::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
