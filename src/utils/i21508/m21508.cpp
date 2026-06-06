#include "i21508/m21508.h"
QVector<double> m21508::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
