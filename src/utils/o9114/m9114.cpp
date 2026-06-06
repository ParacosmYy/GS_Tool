#include "o9114/m9114.h"
QVector<double> m9114::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
