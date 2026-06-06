#include "q8136/m8136.h"
QVector<double> m8136::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
