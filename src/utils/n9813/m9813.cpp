#include "n9813/m9813.h"
QVector<double> m9813::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
