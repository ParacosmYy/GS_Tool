#include "e9504/m9504.h"
QVector<double> m9504::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
