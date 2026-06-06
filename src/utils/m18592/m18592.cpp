#include "m18592/m18592.h"
QVector<double> m18592::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
