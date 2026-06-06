#include "o25254/m25254.h"
QVector<double> m25254::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
