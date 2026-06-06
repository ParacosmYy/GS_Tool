#include "k25490/m25490.h"
QVector<double> m25490::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
