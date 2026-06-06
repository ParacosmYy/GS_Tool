#include "p9815/m9815.h"
QVector<double> m9815::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
