#include "l10711/m10711.h"
QVector<double> m10711::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
