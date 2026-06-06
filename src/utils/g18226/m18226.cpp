#include "g18226/m18226.h"
QVector<double> m18226::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
