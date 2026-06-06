#include "e13004/m13004.h"
QVector<double> m13004::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
