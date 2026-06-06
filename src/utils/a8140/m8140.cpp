#include "a8140/m8140.h"
QVector<double> m8140::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
