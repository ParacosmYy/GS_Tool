#include "k30530/m30530.h"
QVector<double> m30530::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
