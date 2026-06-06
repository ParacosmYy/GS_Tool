#include "o25114/m25114.h"
QVector<double> m25114::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
