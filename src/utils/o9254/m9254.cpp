#include "o9254/m9254.h"
QVector<double> m9254::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
