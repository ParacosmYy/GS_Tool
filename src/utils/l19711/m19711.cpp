#include "l19711/m19711.h"
QVector<double> m19711::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
