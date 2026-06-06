#include "p7815/m7815.h"
QVector<double> m7815::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
